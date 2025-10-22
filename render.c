#include "render.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

void renderInit(app_t *app) {
    setlocale(LC_ALL, "");
    // wprintf(L"\033[8;%d;%dt", RENDER_HEIGHT + 2, MATRIX_WIDTH + 2);
    wprintf(L"\033[H\033[J");
    wprintf(L"\033[?25l");
    setvbuf(stdout, NULL, _IOFBF, BUFSIZ);
    struct termios termios;
    tcgetattr(STDOUT_FILENO, &termios);
    termios.c_lflag &= ~(ICANON | ECHO);
    termios.c_cc[VMIN] = 0;
    termios.c_cc[VTIME] = 0;
    tcsetattr(STDOUT_FILENO, TCSANOW, &termios);
}

void drawMatrix(app_t *app) {
    wprintf(L"\033[H");
    // top bar
    putwchar(L'\u250F');
    for (int i = 0; i < MATRIX_WIDTH; i++)
        putwchar(L'\u2501');
    putwchar(L'\u2513');
    putwchar(L'\n');
    // sides & matrix
    for (int i = RENDER_HEIGHT - 1; i >= 0; i--) {
        putwchar(L'\u2503');
        for (int j = 0; j < MATRIX_WIDTH; j++) {
            if (app->matrix[i][j] != EMPTY) {
                wprintf(L"\033[38;2;%d;%d;%dm", tetTypeColour[app->matrix[i][j] - 1][0], tetTypeColour[app->matrix[i][j] - 1][1], tetTypeColour[app->matrix[i][j] - 1][2]);
                putwchar(L'\u2588');
                wprintf(L"\033[m");
            } else {
                putwchar(L' ');
            }
        }
        putwchar(L'\u2503');
        putwchar(L'\n');
    }
    // bottom bar
    putwchar(L'\u2517');
    for (int i = 0; i < MATRIX_WIDTH; i++)
        putwchar(L'\u2501');
    putwchar(L'\u251B');
}

void drawGhost(app_t *app) {
    int i;
    for (i = 1;; i++) {
        bool canMoveDown = true;
        for (int j = 0; j < 4; j++) {
            if (app->currentTetSquares[j][1] - i < 0 || app->matrix[app->currentTetSquares[j][1] - i][app->currentTetSquares[j][0]] != EMPTY) {
                canMoveDown = false;
                break;
            }
        }
        if (!canMoveDown) break;
    }
    i--;
    wprintf(L"\0337");
    for (int j = 0; j < 4; j++) {
        if (app->currentTetSquares[j][1] - i >= RENDER_HEIGHT) continue;
        wprintf(L"\033[%d;%dH", 1 + RENDER_HEIGHT - app->currentTetSquares[j][1] + i, 2 + app->currentTetSquares[j][0]);
        wprintf(L"\033[38;2;%d;%d;%dm", tetTypeColour[app->currentTetType - 1][0], tetTypeColour[app->currentTetType - 1][1], tetTypeColour[app->currentTetType - 1][2]);
        putwchar(L'\u2592');
    }
    wprintf(L"\0338");
}

void drawCurrentTet(app_t *app) {
    wprintf(L"\0337");
    for (int i = 0; i < 4; i++) {
        if (app->currentTetSquares[i][1] >= RENDER_HEIGHT) continue;
        wprintf(L"\033[%d;%dH", 1 + RENDER_HEIGHT - app->currentTetSquares[i][1], 2 + app->currentTetSquares[i][0]);
        wprintf(L"\033[38;2;%d;%d;%dm", tetTypeColour[app->currentTetType - 1][0], tetTypeColour[app->currentTetType - 1][1], tetTypeColour[app->currentTetType - 1][2]);
        putwchar(L'\u2588');
    }
    wprintf(L"\0338");
}

unsigned char statsTopLeft[2] = {2, MATRIX_WIDTH + 3};

void drawStats(app_t *app) {
    wprintf(L"\0337");
    wprintf(L"\033[%d;%dH", statsTopLeft[0], statsTopLeft[1]);
    wprintf(L"Level: %u", app->level);
    wprintf(L"\033[%d;%dH", statsTopLeft[0] + 1, statsTopLeft[1]);
    wprintf(L"Lines Cleared: %u", app->linesCleared);
    wprintf(L"\033[%d;%dH", statsTopLeft[0] + 2, statsTopLeft[1]);
    wprintf(L"Score: %'u", app->score);
    wprintf(L"\033[%d;%dH", statsTopLeft[0] + 3, statsTopLeft[1]);

    unsigned int minutes, seconds, miliseconds;
    minutes = app->gameTimeElapsed.tv_sec / 60;
    seconds = app->gameTimeElapsed.tv_sec % 60;
    miliseconds = app->gameTimeElapsed.tv_nsec / 1'000'000;
    if (!app->paused) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        struct timespec elapsed = {now.tv_sec - app->lastGameTimeIntervalStart.tv_sec, now.tv_nsec - app->lastGameTimeIntervalStart.tv_nsec};
        if (elapsed.tv_nsec < 0) {
            elapsed.tv_nsec += 1'000'000'000; elapsed.tv_sec -= 1;
        }
        miliseconds += elapsed.tv_nsec / 1'000'000;
        if (miliseconds >= 1000) {
            miliseconds -= 1000;
            seconds += 1;
        }
        seconds += elapsed.tv_sec % 60;
        if (seconds >= 60) {
            seconds -= 60;
            minutes += 1;
        }
        minutes += elapsed.tv_sec / 60;
    }
    wprintf(L"Game Time: %u:%02u:%03u", minutes, seconds, miliseconds);

    wprintf(L"\033[%d;%dH", statsTopLeft[0] + 4, statsTopLeft[1]);
    wprintf(L"Next:");
    const unsigned char nextTetCenter[2] = {statsTopLeft[0] + 6, statsTopLeft[1] + 1};
    for (int i = -1; i <= 0; i++)
        for (int j = -1; j < 3; j++) {
            wprintf(L"\033[%d;%dH", nextTetCenter[0] + i, nextTetCenter[1] + j);
            putwchar(L' ');
        }
    if (!app->paused)
        for (int i = 0; i < 4; i++) {
            wprintf(L"\033[%d;%dH", nextTetCenter[0] - tetPositionTable[app->nextTets[app->nextIndex] - 1][NORTH][i][1], nextTetCenter[1] + tetPositionTable[app->nextTets[app->nextIndex] - 1][NORTH][i][0]);
            wprintf(L"\033[38;2;%d;%d;%dm", tetTypeColour[app->nextTets[app->nextIndex] - 1][0], tetTypeColour[app->nextTets[app->nextIndex] - 1][1], tetTypeColour[app->nextTets[app->nextIndex] - 1][2]);
            putwchar(L'\u2588');
        }

    wprintf(L"\033[%d;%dH", statsTopLeft[0] + 7, statsTopLeft[1]);
    wprintf(L"\033[m");
    wprintf(L"Hold:");
    const unsigned char holdTetCenter[2] = {statsTopLeft[0] + 9, statsTopLeft[1] + 1};
    for (int i = -1; i <= 0; i++)
        for (int j = -1; j < 3; j++) {
            wprintf(L"\033[%d;%dH", holdTetCenter[0] + i, holdTetCenter[1] + j);
            wprintf(L"\033m");
            putwchar(L' ');
        }
    if (!app->paused && app->holdTetType != EMPTY)
        for (int i = 0; i < 4; i++) {
            wprintf(L"\033[%d;%dH", holdTetCenter[0] - tetPositionTable[app->holdTetType - 1][NORTH][i][1], holdTetCenter[1] + tetPositionTable[app->holdTetType - 1][NORTH][i][0]);
            wprintf(L"\033[38;2;%d;%d;%dm", tetTypeColour[app->holdTetType - 1][0], tetTypeColour[app->holdTetType - 1][1], tetTypeColour[app->holdTetType - 1][2]);
            putwchar(L'\u2588');
        }
    wprintf(L"\0338");
}

void actionNotification(app_t *app) {
    wprintf(L"\0337");
    wprintf(L"\033[%d;%dH", statsTopLeft[0] + 11, statsTopLeft[1]);
    for (int i = 0; i < 34; i++) putwchar(L' ');
    if (app->notificationCount < TARGET_FPS)
    {
        wprintf(L"\033[%d;%dH", statsTopLeft[0] + 11, statsTopLeft[1]);
        if (app->notification & BACK_TO_BACK) wprintf(L"Back-to-Back ");
        switch ((char)(app->notification&~BACK_TO_BACK)) {
        case TETRIS:
            wprintf(L"Tetris!");
            break;
        case T_SPIN:
            wprintf(L"T-Spin!");
            break;
        case T_SPIN_SINGLE:
            wprintf(L"T-Spin Single!");
            break;
        case T_SPIN_DOUBLE:
            wprintf(L"T-Spin Double!");
            break;
        case T_SPIN_TRIPLE:
            wprintf(L"T-Spin Triple!");
            break;
        case MINI_T_SPIN:
            wprintf(L"Mini T-Spin!");
            break;
        case MINI_T_SPIN_SINGLE:
            wprintf(L"Mini T-Spin Single!");
            break;
        }
        app->notificationCount++;
    } else {
        app->notification = 0;
        app->notificationCount = 0;
    }
    wprintf(L"\0338");
}

void pauseScreen(app_t *app) {
    wprintf(L"\033[H");
    // top bar
    putwchar(L'\u250F');
    for (int i = 0; i < MATRIX_WIDTH; i++)
        putwchar(L'\u2501');
    putwchar(L'\u2513');
    putwchar(L'\n');
    // sides & matrix
    for (int i = RENDER_HEIGHT - 1; i >= 0; i--) {
        putwchar(L'\u2503');
        for (int j = 0; j < MATRIX_WIDTH; j++) {
            putwchar(L'\u2588');
        }
        putwchar(L'\u2503');
        putwchar(L'\n');
    }
    // bottom bar
    putwchar(L'\u2517');
    for (int i = 0; i < MATRIX_WIDTH; i++)
        putwchar(L'\u2501');
    putwchar(L'\u251B');
}

void render(app_t *app) {
    if (app->paused) {
        pauseScreen(app);
    } else {
        drawMatrix(app);
        if (app->ghost) drawGhost(app);
        drawCurrentTet(app);
    }
    drawStats(app);
    if(app->notification) actionNotification(app);
    fflush(stdout);
}
