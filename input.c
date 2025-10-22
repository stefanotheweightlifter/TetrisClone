#include "input.h"
#include <stddef.h>
#include <unistd.h>

bool canCurrentTetMoveDown(app_t *app);
bool canCurrentTetMoveLeft(app_t *app);
bool canCurrentTetMoveRight(app_t *app);
int canCurrentTetRotate(app_t *app, bool counterClockWise);
void moveCurrentTetDown(app_t *app);
void moveCurrentTetLeft(app_t *app);
void moveCurrentTetRight(app_t *app);
void rotateCurrentTet(app_t *app, int center, bool counterClockWise);
void swapHoldTet(app_t *app);

void placeCurrentTet(app_t *);
void clearLines(app_t *);
void nextToCurrentTet(app_t *);

void handleInput(app_t *app) {
    char buf;
    ssize_t ret = read(STDOUT_FILENO, &buf, 1);
    if (ret == 0) return;
    app->softDropping = false;
    switch (buf) {
    case 'a':
        if (canCurrentTetMoveLeft(app)) moveCurrentTetLeft(app);
        break;
    case 'd':
        if (canCurrentTetMoveRight(app)) moveCurrentTetRight(app);
        break;
    case '\033':
        ret = read(STDOUT_FILENO, &buf, 1);
        if (ret != 0) return;
        app->paused = !app->paused;
        if (app->paused) {
            struct timespec now;
            clock_gettime(CLOCK_MONOTONIC, &now);
            struct timespec elapsed = {now.tv_sec-app->lastGameTimeIntervalStart.tv_sec, now.tv_nsec-app->lastGameTimeIntervalStart.tv_nsec};
            if (elapsed.tv_nsec<0) {
                elapsed.tv_nsec += 1'000'000'000;
                elapsed.tv_sec -= 1;
            }
            app->gameTimeElapsed.tv_nsec += elapsed.tv_nsec;
            if (app->gameTimeElapsed.tv_nsec >= 1'000'000'000) {
                app->gameTimeElapsed.tv_nsec -= 1'000'000'000;
                app->gameTimeElapsed.tv_sec += 1;
            }
            app->gameTimeElapsed.tv_sec += elapsed.tv_sec;
        } else {
            clock_gettime(CLOCK_MONOTONIC, &app->lastGameTimeIntervalStart);
        }
        break;
    case 'g':
        app->ghost = !app->ghost;
        break;
    case 'w': {
        int i;
        for (i = 0; canCurrentTetMoveDown(app); i++)
            moveCurrentTetDown(app);
        app->canFall = false;
        placeCurrentTet(app);
        clearLines(app);
        nextToCurrentTet(app);
        app->score += 2 * i;
        break;
    }
    case 's':
        app->softDropping = true;
        break;
    case 'c':
        if (!app->holdExpended) swapHoldTet(app);
        break;
    case '.': {
        const int res = canCurrentTetRotate(app, false);
        if (res >= 0) rotateCurrentTet(app, res, false);
        break;
    }
    case ',': {
        const int res = canCurrentTetRotate(app, true);
        if (res >= 0) rotateCurrentTet(app, res, true);
        break;
    }
    }
}