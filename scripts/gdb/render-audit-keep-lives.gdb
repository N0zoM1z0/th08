set pagination off
set confirm off
set breakpoint pending on
set debuginfod enabled off
set print thread-events off

# Direct replay starts can enter a later stage without the heap history on
# which the original CopyEnemyNameTexture out-of-bounds read accidentally
# relies.  The copied boss-name strip is unrelated to the enemy draw oracle;
# suppress only that cosmetic copy in this external test harness.
break th08::Gui::CopyEnemyNameTexture(int)
commands
    silent
    printf "render-audit: suppressed direct-stage enemy-name texture copy\n"
    return
    continue
end

break th08::GameManager::AddLives(int) if lives < 0
commands
    silent
    printf "render-audit: suppressed AddLives(%d)\n", lives
    return
    continue
end

run
