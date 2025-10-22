#include "gameLogic.h"

#include <math.h>
#include <memory.h>
#include <stdlib.h>

#include "input.h"

void randomBag(app_t *app) {
    char types[7];
    for(int i = 0; i < 7; i++) types[i] = i + 1;
    for(int i = 0; i < 6; i++) {
        int const r = rand() % (7 - i);
        app->nextTets[7 + i] = types[r];
        memmove(types + r, types + r + 1, 6 - i - r);
    }
    app->nextTets[7 + 6] = types[0];
}

bool canCurrentTetMoveDown(app_t *app) {
    for(int i = 0; i < 4; i++)
        if(app->currentTetSquares[i][1] - 1 < 0 || app->matrix[app->currentTetSquares[i][1] - 1][app->currentTetSquares[i][0]] != EMPTY) return false;
    return true;
}

bool canCurrentTetMoveLeft(app_t *app) {
    for(int i = 0; i < 4; i++)
        if(app->currentTetSquares[i][0] - 1 == -1 || app->matrix[app->currentTetSquares[i][1]][app->currentTetSquares[i][0] - 1] != EMPTY) return false;
    return true;
}

bool canCurrentTetMoveRight(app_t *app) {
    for(int i = 0; i < 4; i++)
        if(app->currentTetSquares[i][0] + 1 == MATRIX_WIDTH || app->matrix[app->currentTetSquares[i][1]][app->currentTetSquares[i][0] + 1] != EMPTY) return false;
    return true;
}

int canCurrentTetRotate(app_t *app, bool counterClockWise) {
    if(app->currentTetType == TET_O) return -1;
    char const newRot = (app->currentTetOrientation + (counterClockWise ? 3 : 1)) % 4;
    char newPlaces[4][2];
    int i;
    for(i = 0; i < 5; i++) {
        for(int j = 0; j < 4; j++) {
            newPlaces[j][0] = app->currentTetPosition[0] + tetPositionTable[app->currentTetType - 1][newRot][j][0] +
                              (i ? tetRotationPoints[app->currentTetType > TET_I][app->currentTetOrientation][i - 1][0] - tetRotationPoints[app->currentTetType > TET_I][newRot][i - 1][0] : 0);
            newPlaces[j][1] = app->currentTetPosition[1] + tetPositionTable[app->currentTetType - 1][newRot][j][1] +
                              (i ? tetRotationPoints[app->currentTetType > TET_I][app->currentTetOrientation][i - 1][1] - tetRotationPoints[app->currentTetType > TET_I][newRot][i - 1][1] : 0);
        }
        bool valid = true;
        for(int j = 0; j < 4; j++) {
            if(newPlaces[j][0] < 0 || newPlaces[j][0] >= MATRIX_WIDTH || newPlaces[j][1] < 0 || app->matrix[newPlaces[j][1]][newPlaces[j][0]] != EMPTY) {
                valid = false;
                break;
            }
        }
        if(valid) break;
    }
    if(i == 5) return -1;
    return i;
}

void moveCurrentTetDown(app_t *app) {
    for(int i = 0; i < 4; i++) app->currentTetSquares[i][1]--;
    app->currentTetPosition[1]--;
    app->justRotated = false;
    memset(&app->lastFallTime, 0, sizeof app->lastLockdownTime);
    memset(&app->lastLockdownTime, 0, sizeof app->lastLockdownTime);
}

void moveCurrentTetLeft(app_t *app) {
    for(int i = 0; i < 4; i++) app->currentTetSquares[i][0]--;
    app->currentTetPosition[0]--;
    app->justRotated = false;
    memset(&app->lastLockdownTime, 0, sizeof app->lastLockdownTime);
}

void moveCurrentTetRight(app_t *app) {
    for(int i = 0; i < 4; i++) app->currentTetSquares[i][0]++;
    app->currentTetPosition[0]++;
    app->justRotated = false;
    memset(&app->lastLockdownTime, 0, sizeof app->lastLockdownTime);
}

void rotateCurrentTet(app_t *app, int center, bool counterClockWise) {
    char const newRot = (app->currentTetOrientation + (counterClockWise ? 3 : 1)) % 4;
    for(int i = 0; i < 4; i++) {
        app->currentTetSquares[i][0] =
            app->currentTetPosition[0] + tetPositionTable[app->currentTetType - 1][newRot][i][0] +
            (center ? tetRotationPoints[app->currentTetType > TET_I][app->currentTetOrientation][center - 1][0] - tetRotationPoints[app->currentTetType > TET_I][newRot][center - 1][0] : 0);
        app->currentTetSquares[i][1] =
            app->currentTetPosition[1] + tetPositionTable[app->currentTetType - 1][newRot][i][1] +
            (center ? tetRotationPoints[app->currentTetType > TET_I][app->currentTetOrientation][center - 1][1] - tetRotationPoints[app->currentTetType > TET_I][newRot][center - 1][1] : 0);
    }
    app->currentTetPosition[0] += center ? tetRotationPoints[app->currentTetType > TET_I][app->currentTetOrientation][center - 1][0] - tetRotationPoints[app->currentTetType > TET_I][newRot][center - 1][0] : 0;
    app->currentTetPosition[1] += center ? tetRotationPoints[app->currentTetType > TET_I][app->currentTetOrientation][center - 1][1] - tetRotationPoints[app->currentTetType > TET_I][newRot][center - 1][1] : 0;
    app->currentTetOrientation = newRot;
    app->justRotated = true;
    memset(&app->lastLockdownTime, 0, sizeof app->lastLockdownTime);
}

void reset(app_t *app);

void setTypeAsCurrentTet(app_t *app, char tetType) {
    app->currentTetOrientation = NORTH;
    app->currentTetPosition[0] = tetStartPosition[0];
    app->currentTetPosition[1] = tetStartPosition[1];
    app->currentTetType = tetType;
    for(int i = 0; i < 4; i++) {
        if(app->matrix[app->currentTetSquares[i][1]]) app->currentTetSquares[i][0] = tetStartPosition[0] + tetPositionTable[tetType - 1][NORTH][i][0];
        app->currentTetSquares[i][1] = tetStartPosition[1] + tetPositionTable[tetType - 1][NORTH][i][1];
    }
    app->canFall = true;
    memset(&app->lastFallTime, 0, sizeof(app->lastFallTime));
    memset(&app->lastFallTime, 0, sizeof(app->lastLockdownTime));
}

void nextToCurrentTet(app_t *app) {
    setTypeAsCurrentTet(app, app->nextTets[app->nextIndex]);
    app->nextIndex++;
    if(app->nextIndex == 7) {
        memmove(app->nextTets, app->nextTets + 7, 7 * sizeof *app->nextTets);
        randomBag(app);
        app->nextIndex = 0;
    }
}

void swapHoldTet(app_t *app) {
    if(app->holdTetType != EMPTY) {
        char const temp = app->holdTetType;
        app->holdTetType = app->currentTetType;
        setTypeAsCurrentTet(app, temp);
    } else {
        app->holdTetType = app->currentTetType;
        nextToCurrentTet(app);
    }
    app->holdExpended = true;
}

void placeCurrentTet(app_t *app) {
    for(int i = 0; i < 4; i++) app->matrix[app->currentTetSquares[i][1]][app->currentTetSquares[i][0]] = app->currentTetType;
    app->holdExpended = false;
}

void levelUp(app_t *app) {
    app->level++;
    app->fallSpeed = powf(.8f - .007f * (app->level - 1), app->level - 1);
}

void actionNotification(app_t *app, char action);

void clearLines(app_t *app) {
    char action = 0;
    int linesCleared = 0;
    int scoreToAdd = 0;
    bool backToBack = false;
    unsigned char isTSpin = 0;
    if(app->currentTetType == TET_T && app->justRotated)
        for(int i = 0; i < 4; i++)
            isTSpin |= (app->currentTetPosition[0] + tSpinChecks[app->currentTetOrientation][i][0] < 0 || app->currentTetPosition[0] + tSpinChecks[app->currentTetOrientation][i][0] >= MATRIX_WIDTH ||
                        app->currentTetPosition[1] + tSpinChecks[app->currentTetOrientation][i][1] < 0 ||
                        app->matrix[app->currentTetPosition[1] + tSpinChecks[app->currentTetOrientation][i][1]][app->currentTetPosition[0] + tSpinChecks[app->currentTetOrientation][i][0]] != EMPTY)
                       << i;
    for(int i = 0; i < MATRIX_HEIGHT;) {
        bool clearRow = true;
        for(int j = 0; j < MATRIX_WIDTH; j++) {
            if(app->matrix[i][j] == EMPTY) {
                clearRow = false;
                break;
            }
        }
        if(clearRow) {
            linesCleared++;
            memmove(&app->matrix[i][0], &app->matrix[i + 1][0], (MATRIX_HEIGHT - 1 - i) * MATRIX_WIDTH * sizeof **app->matrix);
            memset(&app->matrix[MATRIX_HEIGHT - 1][0], EMPTY, MATRIX_WIDTH * sizeof **app->matrix);
        } else {
            i++;
        }
    }
    if(app->currentTetType == TET_T && app->justRotated) {
        if((isTSpin & 0b11) == 0b11 && isTSpin & 0b1100) {  // t spin
            scoreToAdd = 400 * (linesCleared + 1) * app->level;
            if(app->canBackToBack && linesCleared) backToBack = true;
            if(linesCleared) app->canBackToBack = true;
            switch(linesCleared) {
            case 0: action = T_SPIN; break;
            case 1: action = T_SPIN_SINGLE; break;
            case 2: action = T_SPIN_DOUBLE; break;
            case 3: action = T_SPIN_TRIPLE; break;
            }
        } else if(isTSpin & 0b11 && (isTSpin & 0b1100) == 0b1100) {  // t spin mini
            scoreToAdd = 100 * (linesCleared + 1) * app->level;
            if(app->canBackToBack && linesCleared) backToBack = true;
            if(linesCleared) app->canBackToBack = true;
            action = linesCleared ? MINI_T_SPIN_SINGLE : MINI_T_SPIN;
        } else goto regularLineClear;
    } else {
    regularLineClear:
        switch(linesCleared) {
        case 1:
            scoreToAdd = 100 * app->level;
            app->canBackToBack = false;
            break;
        case 2:
            scoreToAdd = 300 * app->level;
            app->canBackToBack = false;
            break;
        case 3:
            scoreToAdd = 500 * app->level;
            app->canBackToBack = false;
            break;
        case 4:
            scoreToAdd = 800 * app->level;
            if(app->canBackToBack) backToBack = true;
            app->canBackToBack = true;
            action = TETRIS;
            break;
        }
    }
    app->linesCleared += linesCleared;
    app->score += scoreToAdd;
    if(backToBack) {
        app->score += scoreToAdd / 2;
        action |= BACK_TO_BACK;
    }
    if(app->linesCleared >= 10 * app->level) levelUp(app);
    if(action) {
        app->notification = action;
        app->notificationCount = 0;
    }
}

void reset(app_t *app) {
    memset(app->matrix, EMPTY, sizeof(app->matrix));
    app->paused = false;
    clock_gettime(CLOCK_MONOTONIC, &app->lastGameTimeIntervalStart);
    memset(&app->gameTimeElapsed, 0, sizeof(struct timespec));
    randomBag(app);
    memmove(app->nextTets, app->nextTets + 7, 7 * sizeof *app->nextTets);
    randomBag(app);
    app->nextIndex = 0;
    app->holdTetType = EMPTY;
    nextToCurrentTet(app);
    app->canFall = false;
    app->justRotated = false;
    app->canBackToBack = false;
    app->holdExpended = false;
    app->level = 0;
    levelUp(app);
    memset(&app->lastFallTime, 0, sizeof(app->lastFallTime));
    app->linesCleared = 0;
    app->score = 0;
    memset(&app->lastFallTime, 0, sizeof(app->lastLockdownTime));
}

void gameInit(app_t *app) {
    app->lastGameTimeIntervalStart = app->startTime;
    levelUp(app);
    randomBag(app);
    memmove(app->nextTets, app->nextTets + 7, 7 * sizeof *app->nextTets);
    randomBag(app);
    nextToCurrentTet(app);
    app->ghost = true;
}

void gameLogic(app_t *app) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    handleInput(app);
    if(app->paused) return;
    app->canFall = canCurrentTetMoveDown(app);
    if(app->canFall) {
        if(memcmp(&app->lastFallTime, &(struct timespec){0, 0}, sizeof app->lastFallTime) == 0) {
            app->lastFallTime = now;
            return;
        }
        float fallDuration = now.tv_sec - app->lastFallTime.tv_sec + (now.tv_nsec - app->lastFallTime.tv_nsec) / 1e9f;
        if(app->softDropping && fallDuration >= app->fallSpeed / 10 || fallDuration >= app->fallSpeed) {
            moveCurrentTetDown(app);
            memset(&app->lastFallTime, 0, sizeof(app->lastFallTime));
            app->score += app->softDropping;
        }
    } else {
        memset(&app->lastFallTime, 0, sizeof(app->lastFallTime));
        if(memcmp(&app->lastLockdownTime, &(struct timespec){0, 0}, sizeof app->lastLockdownTime) == 0) {
            app->lastLockdownTime = now;
            return;
        }
        float lockdownDuration = now.tv_sec - app->lastLockdownTime.tv_sec + (now.tv_nsec - app->lastLockdownTime.tv_nsec) / 1e9f;
        if(lockdownDuration >= LOCK_DOWN_TIME) {
            placeCurrentTet(app);
            clearLines(app);
            nextToCurrentTet(app);
        }
    }
}