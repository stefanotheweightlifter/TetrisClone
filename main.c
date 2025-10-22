#include "app.h"
#include "mainLoop.h"
#include <stdio.h>
#include <signal.h>
#include<stdlib.h>

void cleanup(int) {
    system("reset");
    exit(0);
}

int main() {
    sigaction(
        SIGINT,
        &(struct sigaction){
            .sa_handler = cleanup,
            .sa_mask = 0,
            .sa_flags = 0
        },
        NULL);
    app_t app = {};
    clock_gettime(CLOCK_MONOTONIC, &app.startTime);
    mainLoop(&app);
}
