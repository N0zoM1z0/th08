set pagination off
set confirm off
set breakpoint pending on
set debuginfod enabled off
set print thread-events off

# Keep the product boss-name copy; allow the historical bypass for diagnostics.
python
import gdb, os
if os.environ.get('TH08_RENDER_AUDIT_SKIP_ENEMY_NAME') == '1':
    gdb.execute('break th08::Gui::CopyEnemyNameTexture(int)')
    gdb.execute('commands\nsilent\nprintf "render-audit: suppressed enemy-name texture copy\\n"\nreturn\ncontinue\nend')
end

break th08::GameManager::AddLives(int) if lives < 0
commands
    silent
    printf "render-audit: suppressed AddLives(%d)\n", lives
    return
    continue
end

run
