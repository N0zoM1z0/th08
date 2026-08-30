#include "th_pch.h"

#include "EnemyManager.hpp"
#include "GameManager.hpp"
#include "modern/linux/d3d8_internal.hpp"
#include "modern/linux/render_audit.hpp"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace th08
{
namespace modern
{
namespace
{
bool AuditEnvironmentEnabled()
{
    const char *value = getenv("TH08_RENDER_AUDIT");
    if (value == NULL || value[0] == '\0')
        value = getenv("TH08_LINUX_RENDER_AUDIT");
    return value != NULL && value[0] != '\0' && strcmp(value, "0") != 0;
}

const char *AuditOutputPath()
{
    const char *path = getenv("TH08_RENDER_AUDIT_FILE");
    if (path == NULL || path[0] == '\0')
        path = getenv("TH08_LINUX_RENDER_AUDIT_FILE");
    return path != NULL && path[0] != '\0' ? path : "modern-enemy-render.csv";
}

u8 MixRenderColorChannel(u8 first, u8 second)
{
    const u32 mixed = first * second / 128U;
    return mixed > 255U ? 255U : static_cast<u8>(mixed);
}

ZunColor RenderColorForVm(const AnmVm &vm)
{
    ZunColor color = vm.flag17 ? vm.color2 : vm.color1;
    if (g_AnmManager->useMixColor)
    {
        color.r = MixRenderColorChannel(color.r, g_AnmManager->color.r);
        color.g = MixRenderColorChannel(color.g, g_AnmManager->color.g);
        color.b = MixRenderColorChannel(color.b, g_AnmManager->color.b);
        color.a = MixRenderColorChannel(color.a, g_AnmManager->color.a);
    }
    return color;
}

void WriteAuditRow(Enemy *enemy, ZunResult drawResult, const char *status,
                   const LinuxTextureRegionStats &source,
                   const LinuxFramebufferDeltaStats &delta, bool sourceAvailable,
                   bool probeAvailable, ZunColor renderColor,
                   float screenX, float screenY)
{
    FILE *file = fopen(AuditOutputPath(), "ab+");
    if (file == NULL)
        return;
    fseek(file, 0, SEEK_END);
    if (ftell(file) == 0)
    {
        fputs("schema_version,stage,frame,enemy_index,boss,draw_group,script,sprite,anm_file,loaded_anm,"
              "visible,draw_enabled,flag17,color1,color2,render_color,x,y,width,height,"
              "anchor,rotation,screen_x,screen_y,u0,v0,u1,v1,"
              "source_pixels,source_visible,source_colorful,source_white,source_edge,"
              "expected_visible,expected_colorful,expected_white,"
              "draw_result,probe_pixels,changed_pixels,changed_colorful,changed_chromatic,changed_white,"
              "absolute_rgb_difference,status\n", file);
    }

    const AnmVm &vm = enemy->vm;
    const AnmLoadedSprite *sprite = vm.loadedSprite;
    const int enemyIndex = static_cast<int>(enemy - &g_EnemyManager.enemies[0]);
    fprintf(file,
            "1,%d,%u,%d,%u,%u,%d,%d,%d,%d,%u,%u,%u,%08x,%08x,%08x,"
            "%.3f,%.3f,%.3f,%.3f,%u,%.7f,%.3f,%.3f,%.7f,%.7f,%.7f,%.7f,"
            "%u,%u,%u,%u,%u,%u,%u,%u,%d,%u,%u,%u,%u,%u,%llu,%s\n",
            g_GameManager.currentStage, g_GameManager.stageActiveFrames, enemyIndex,
            (enemy->flags1 & ENEMY_FLAG_BOSS) != 0, enemy->drawGroup,
            vm.scriptIndex, vm.activeSpriteIndex, vm.anmFileIndex,
            sprite != NULL ? sprite->anmIdx : -1,
            static_cast<unsigned int>(vm.visible), static_cast<unsigned int>(vm.flag1),
            static_cast<unsigned int>(vm.flag17), vm.color1.d3dColor, vm.color2.d3dColor,
            renderColor.d3dColor, vm.pos.x, vm.pos.y,
            vm.spriteSize.x * vm.scale.x, vm.spriteSize.y * vm.scale.y,
            static_cast<unsigned int>(vm.anchor), vm.rotation.z, screenX, screenY,
            sprite != NULL ? sprite->uvStart.x + vm.uvScrollPos.x : 0.0f,
            sprite != NULL ? sprite->uvStart.y + vm.uvScrollPos.y : 0.0f,
            sprite != NULL ? sprite->uvEnd.x + vm.uvScrollPos.x : 0.0f,
            sprite != NULL ? sprite->uvEnd.y + vm.uvScrollPos.y : 0.0f,
            sourceAvailable ? source.sampledPixels : 0,
            sourceAvailable ? source.visiblePixels : 0,
            sourceAvailable ? source.colorfulPixels : 0,
            sourceAvailable ? source.nearWhitePixels : 0,
            sourceAvailable ? source.visibleEdgePixels : 0,
            sourceAvailable ? source.modulatedVisiblePixels : 0,
            sourceAvailable ? source.modulatedColorfulPixels : 0,
            sourceAvailable ? source.modulatedNearWhitePixels : 0,
            static_cast<int>(drawResult), probeAvailable ? delta.sampledPixels : 0,
            probeAvailable ? delta.changedPixels : 0,
            probeAvailable ? delta.colorfulChangedPixels : 0,
            probeAvailable ? delta.chromaticChangedPixels : 0,
            probeAvailable ? delta.nearWhiteChangedPixels : 0,
            probeAvailable ? delta.absoluteRgbDifference : 0, status);
    fclose(file);
}
} // namespace

bool IsEnemyRenderAuditEnabled()
{
    static const bool enabled = AuditEnvironmentEnabled();
    return enabled;
}

ZunResult AuditEnemyPrimaryDraw(Enemy *enemy)
{
    const int enemyIndex = static_cast<int>(enemy - &g_EnemyManager.enemies[0]);
    const bool isBoss = (enemy->flags1 & ENEMY_FLAG_BOSS) != 0;
    const unsigned int samplePeriod = isBoss ? 15U : 60U;
    if ((g_GameManager.stageActiveFrames + enemyIndex) % samplePeriod != 0)
        return g_AnmManager->Draw2D(&enemy->vm);

    AnmVm &vm = enemy->vm;
    const ZunColor renderColor = RenderColorForVm(vm);
    LinuxTextureRegionStats source;
    LinuxFramebufferDeltaStats delta;
    memset(&source, 0, sizeof(source));
    memset(&delta, 0, sizeof(delta));

    const bool hasSprite = vm.loadedSprite != NULL;
    const bool hasTexture = hasSprite && vm.loadedSprite->texture != NULL;
    const float width = fabsf(vm.spriteSize.x * vm.scale.x);
    const float height = fabsf(vm.spriteSize.y * vm.scale.y);
    const float signedHalfWidth = vm.spriteSize.x * vm.scale.x * 0.5f;
    const float signedHalfHeight = vm.spriteSize.y * vm.scale.y * 0.5f;
    const float sine = fabsf(sinf(vm.rotation.z));
    const float cosine = fabsf(cosf(vm.rotation.z));
    const float halfExtentX = fabsf(signedHalfWidth) * cosine +
                              fabsf(signedHalfHeight) * sine;
    const float halfExtentY = fabsf(signedHalfWidth) * sine +
                              fabsf(signedHalfHeight) * cosine;
    float screenX = vm.pos.x + g_AnmManager->screenShakeOffset.x;
    float screenY = vm.pos.y + g_AnmManager->screenShakeOffset.y;
    if (vm.anchor & 1)
        screenX += signedHalfWidth;
    if (vm.anchor & 2)
        screenY += signedHalfHeight;
    const bool validGeometry = width > 0.0f && height > 0.0f &&
                               width < 4096.0f && height < 4096.0f;
    const bool sourceAvailable = hasTexture &&
        th08_linux_texture_region_stats(vm.loadedSprite->texture,
                                        vm.loadedSprite->uvStart.x + vm.uvScrollPos.x,
                                        vm.loadedSprite->uvStart.y + vm.uvScrollPos.y,
                                        vm.loadedSprite->uvEnd.x + vm.uvScrollPos.x,
                                        vm.loadedSprite->uvEnd.y + vm.uvScrollPos.y,
                                        renderColor.d3dColor, &source);

    bool intersectsViewport = false;
    bool fullyInsideViewport = false;
    bool probeStarted = false;
    if (hasTexture && validGeometry && vm.visible && vm.flag1 && vm.color1.a != 0)
    {
        const int left = static_cast<int>(floorf(screenX - halfExtentX)) - 1;
        const int top = static_cast<int>(floorf(screenY - halfExtentY)) - 1;
        const int right = static_cast<int>(ceilf(screenX + halfExtentX)) + 1;
        const int bottom = static_cast<int>(ceilf(screenY + halfExtentY)) + 1;
        const int viewportLeft = g_Supervisor.viewport.X;
        const int viewportTop = g_Supervisor.viewport.Y;
        const int viewportRight = viewportLeft + g_Supervisor.viewport.Width;
        const int viewportBottom = viewportTop + g_Supervisor.viewport.Height;
        intersectsViewport = left < viewportRight && right > viewportLeft &&
                             top < viewportBottom && bottom > viewportTop;
        fullyInsideViewport = left >= viewportLeft && right <= viewportRight &&
                              top >= viewportTop && bottom <= viewportBottom;
        if (intersectsViewport)
        {
            const int clippedLeft = left > viewportLeft ? left : viewportLeft;
            const int clippedTop = top > viewportTop ? top : viewportTop;
            const int clippedRight = right < viewportRight ? right : viewportRight;
            const int clippedBottom = bottom < viewportBottom ? bottom : viewportBottom;
            g_AnmManager->FlushVertexBuffer();
            probeStarted = th08_linux_begin_framebuffer_probe(
                g_Supervisor.d3dDevice, clippedLeft, clippedTop, clippedRight, clippedBottom);
        }
    }

    ZunResult result = ZUN_ERROR;
    if (hasSprite)
        result = g_AnmManager->Draw2D(&vm);

    bool probeAvailable = false;
    if (probeStarted)
    {
        g_AnmManager->FlushVertexBuffer();
        probeAvailable = th08_linux_end_framebuffer_probe(g_Supervisor.d3dDevice, &delta);
    }

    const char *status = "ok";
    if (!hasSprite)
        status = "missing-sprite";
    else if (!hasTexture)
        status = "missing-texture";
    else if (!validGeometry)
        status = "invalid-geometry";
    else if (!sourceAvailable)
        status = "source-unavailable";
    else if (source.visiblePixels == 0)
        status = "empty-source";
    else if (result != ZUN_SUCCESS)
        status = "not-queued";
    else if (!intersectsViewport)
        status = "offscreen";
    else if (!probeAvailable)
        status = "unprobed";
    else if (!fullyInsideViewport)
        status = "partially-offscreen";
    else if (source.modulatedVisiblePixels == 0)
        status = "vm-transparent-output";
    else if (source.colorfulPixels * 5U > source.visiblePixels &&
             source.modulatedColorfulPixels == 0)
        status = "vm-tinted-output";
    else if (delta.changedPixels == 0)
        status = "no-pixel-delta";
    else if (source.modulatedColorfulPixels * 5U > source.modulatedVisiblePixels &&
             delta.chromaticChangedPixels == 0)
        status = "color-loss-suspect";
    else if (source.modulatedNearWhitePixels * 5U < source.modulatedVisiblePixels &&
             delta.nearWhiteChangedPixels * 4U > delta.changedPixels * 3U)
        status = "white-output-suspect";

    WriteAuditRow(enemy, result, status, source, delta, sourceAvailable, probeAvailable,
                  renderColor, screenX, screenY);
    return result;
}
} // namespace modern
} // namespace th08
