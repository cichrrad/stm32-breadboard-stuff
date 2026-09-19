#include "mini_game_runner.h"

GameInstance_t *mgr_get_current_game_instance(MiniGameRunner_t *runner)
{
    return &runner->games[runner->currentMinigame];
}

void mgr_run_minigame(MiniGameRunner_t *runner, Minigame_t game)
{
    runner->currentMinigame = game;
    runner->running = true;
}

bool mgr_is_running(MiniGameRunner_t *runner)
{
    return runner->running;
}

void mgr_call_initFn(MiniGameRunner_t *runner)
{
    runner->games[runner->currentMinigame].initFn();
}

void mgr_call_updateFn(MiniGameRunner_t *runner)
{
    runner->games[runner->currentMinigame].updateFn(&(runner->games[runner->currentMinigame].exit_flag));
}

void mgr_call_renderFn(MiniGameRunner_t *runner)
{
    runner->games[runner->currentMinigame].renderFn();
}
