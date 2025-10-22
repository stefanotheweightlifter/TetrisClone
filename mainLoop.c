#include "mainLoop.h"
#include "gameLogic.h"
#include "render.h"

int renderThreadFunc(void *args) {
    app_t *app = (app_t *)args;
    mtx_lock(&app->renderMtx);
    while (true) {
        if (!app->rendering) cnd_wait(&app->renderCnd, &app->renderMtx);
        render(app);
        app->rendering = false;
        cnd_signal(&app->renderCnd);
    }
}

void mainLoop(app_t *app) {
    gameInit(app);
    renderInit(app);
    mtx_init(&app->renderMtx, mtx_plain);
    cnd_init(&app->renderCnd);
    thrd_create(&app->renderThread, &renderThreadFunc, app);
    struct timespec lastFrameTime, currentTime;
    lastFrameTime = app->startTime;
    mtx_lock(&app->renderMtx);
    while (true) {
        gameLogic(app);
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        float duration = currentTime.tv_sec - lastFrameTime.tv_sec + (currentTime.tv_nsec - lastFrameTime.tv_nsec) / 1e9f;
        if (duration >= TARGET_FRAME_TIME) {
            mtx_unlock(&app->renderMtx);
            app->rendering = true;
            cnd_signal(&app->renderCnd);
            if (app->rendering) cnd_wait(&app->renderCnd, &app->renderMtx);
            lastFrameTime = currentTime;
        }
    }
}
