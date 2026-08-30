#pragma once

#ifndef TH08_MODERN_LINUX
#error "The render audit is only available in the modern Linux build."
#endif

#include "ZunResult.hpp"

namespace th08
{
struct Enemy;

namespace modern
{
bool IsEnemyRenderAuditEnabled();
ZunResult AuditEnemyPrimaryDraw(Enemy *enemy);
}
} // namespace th08
