#ifndef TIMER_H
#define TIMER_H

#include "driver/timer.h"

typedef struct timer {
    timer_group_t group;
    timer_idx_t number;
    int counts;
    volatile int triggered;
    void (*callback)(void*);
    void* argument;
} timer;

timer* init_timer(float seconds, int groupNum, int timerNum, void (*timerCallback)(void*), void* arg);
void start_timer(timer* t);
void stop_timer(timer* t);

#endif