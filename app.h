#ifndef APP_H
#define APP_H
#include <time.h>
#include <wchar.h>
#include<threads.h>

#define MATRIX_WIDTH 10
#define MATRIX_HEIGHT 40
#define RENDER_HEIGHT 20
#define TARGET_FPS 120
#define TARGET_FRAME_TIME (1.f / TARGET_FPS)

#define LOCK_DOWN_TIME .5f

enum : char {
    EMPTY,
    TET_O,
    TET_I,
    TET_T,
    TET_L,
    TET_J,
    TET_S,
    TET_Z
};

enum : char {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

enum : unsigned char {
    TETRIS = 1,
    T_SPIN,
    T_SPIN_SINGLE,
    T_SPIN_DOUBLE,
    T_SPIN_TRIPLE,
    MINI_T_SPIN,
    MINI_T_SPIN_SINGLE,
    BACK_TO_BACK = 1<<7
};

unsigned char constexpr tetStartPosition[2] = {4, 20};

unsigned char constexpr tetTypeColour[7][3] = {
    {255, 244, 22},  // O
    {155, 236, 255}, // I
    {209, 161, 255}, // T
    {247, 154, 14},  // L
    {44, 137, 255},  // J
    {31, 255, 0},    // S
    {220, 14, 7}     // Z
};

char constexpr tetPositionTable[7][4][4][2] = {
    {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, // NORTH
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, // EAST
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, // SOUTH
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}}  // WEST
    },                                    //  O
    {
        {{-1, 0}, {0, 0}, {1, 0}, {2, 0}},     // NORTH
        {{1, -2}, {1, -1}, {1, 0}, {1, 1}},    // EAST
        {{-1, -1}, {0, -1}, {1, -1}, {2, -1}}, // SOUTH
        {{0, -2}, {0, -1}, {0, 0}, {0, 1}}     // WEST
    },                                         //  I
    {
        {{-1, 0}, {0, 0}, {1, 0}, {0, 1}},  // NORTH
        {{0, -1}, {0, 0}, {1, 0}, {0, 1}},  // EAST
        {{0, -1}, {-1, 0}, {0, 0}, {1, 0}}, // SOUTH
        {{0, -1}, {-1, 0}, {0, 0}, {0, 1}}, // WEST
    },                                      //  T
    {
        {{-1, 0}, {0, 0}, {1, 0}, {1, 1}},   // NORTH
        {{0, -1}, {1, -1}, {0, 0}, {0, 1}},  // EAST
        {{-1, -1}, {-1, 0}, {0, 0}, {1, 0}}, // SOUTH
        {{0, -1}, {0, 0}, {-1, 1}, {0, 1}},  // WEST
    },                                       //  L
    {
        {{-1, 0}, {0, 0}, {1, 0}, {-1, 1}},  // NORTH
        {{0, -1}, {0, 0}, {0, 1}, {1, 1}},   // EAST
        {{1, -1}, {-1, 0}, {0, 0}, {1, 0}},  // SOUTH
        {{-1, -1}, {0, -1}, {0, 0}, {0, 1}}, // WEST
    },                                       //  J
    {
        {{-1, 0}, {0, 0}, {0, 1}, {1, 1}},   // NORTH
        {{1, -1}, {0, 0}, {1, 0}, {0, 1}},   // EAST
        {{-1, -1}, {0, -1}, {0, 0}, {1, 0}}, // SOUTH
        {{0, -1}, {-1, 0}, {0, 0}, {-1, 1}}, // WEST
    },                                       //  S
    {
        {{0, 0}, {1, 0}, {-1, 1}, {0, 1}},   // NORTH
        {{0, -1}, {0, 0}, {1, 0}, {1, 1}},   // EAST
        {{0, -1}, {1, -1}, {-1, 0}, {0, 0}}, // SOUTH
        {{-1, -1}, {-1, 0}, {0, 0}, {0, 1}}, // WEST
    } //  Z
};

char constexpr tetRotationPoints[2][4][4][2] = {
    {
        {{-1, 0}, {2, 0}, {-1, 0}, {2, 0}},   // NORTH
        {{1, 0}, {1, 0}, {1, 1}, {1, -2}},    // EAST
        {{2, 0}, {-1, 0}, {2, -1}, {-1, -1}}, // SOUTH
        {{0, 0}, {0, 0}, {0, -2}, {0, 1}}     // WEST
    },                                        // I
    {
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}},    // NORTH
        {{1, 0}, {1, -1}, {0, 2}, {1, 2}},   // EAST
        {{0, 0}, {0, 0}, {0, 0}, {0, 0}},    // SOUTH
        {{-1, 0}, {-1, -1}, {0, 2}, {-1, 2}} // WEST
    } // T, L, J, S, Z
};

char constexpr tSpinChecks[4][4][2] = {
    {{-1, 1}, {1, 1}, {-1, -1}, {1, -1}},
    {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}},
    {{1, -1}, {-1, -1}, {1, 1}, {-1, 1}},
    {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};

typedef struct {
    char *name;
    char matrix[MATRIX_HEIGHT][MATRIX_WIDTH];
    struct timespec startTime;

    bool paused;
    struct timespec lastGameTimeIntervalStart;
    struct timespec gameTimeElapsed;
    bool ghost;

    char nextTets[14];
    char nextIndex;
    char holdTetType;
    char currentTetType;
    char currentTetOrientation;
    unsigned char currentTetPosition[2];
    unsigned char currentTetSquares[4][2];

    bool canFall;
    bool justRotated;
    bool canBackToBack;
    bool holdExpended;
    bool softDropping;

    unsigned char level;
    float fallSpeed;
    struct timespec lastFallTime;
    unsigned int linesCleared;
    unsigned int score;

    struct timespec lastLockdownTime;

    bool rendering;
    mtx_t renderMtx;
    cnd_t renderCnd;
    thrd_t renderThread;

    char notification;
    char notificationCount;
} app_t;
#endif
