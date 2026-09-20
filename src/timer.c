#include "timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void IRAM_ATTR timer_isr_handler(void *arg) {
    timer* timerPtr = (timer*) arg;

    timer_group_clr_intr_status_in_isr(timerPtr->group, timerPtr->number); // Clear the current interrupt

    timerPtr->triggered = 1;
    
    timer_group_enable_alarm_in_isr(timerPtr->group, timerPtr->number); // Enable the next alarm
}

timer* init_timer(float seconds, int groupNum, int timerNum, void (*timerCallback)(void*), void* arg) {
    timer_config_t config = {
        .alarm_en = true, 
        .counter_en = false, 
        .intr_type = TIMER_INTR_LEVEL,
        .counter_dir = TIMER_COUNT_UP, 
        .auto_reload = true, 
        .divider = 80
    };

    int counts = seconds * 1000000;

    timer_group_t group = (groupNum) ? TIMER_GROUP_1 : TIMER_GROUP_0;
    timer_idx_t number  = (timerNum) ? TIMER_1 : TIMER_0;

    timer* timerEntity = calloc(1, sizeof(timer));

    timerEntity->group = group;
    timerEntity->number = number;
    timerEntity->counts = counts;
    timerEntity->callback = timerCallback;
    timerEntity->argument = arg;

    timer_init(group, number, &config);
    timer_set_counter_value(group, number, 0);
    timer_set_alarm_value(group, number, counts);

    timer_isr_register(
        group, 
        number, 
        timer_isr_handler, 
        (void*) timerEntity, 
        0, 
        NULL
    ); // Register the interrupt

    return timerEntity;
}

void start_timer(timer* t) {
    timer_enable_intr(t->group, t->number);
    timer_start(t->group, t->number);
}

void stop_timer(timer* t) {
    timer_disable_intr(t->group, t->number);
    timer_pause(t->group, t->number);
}

void update_timer_period(timer* t, float seconds) {
    timer_set_alarm_value(t->group, t->number, seconds * 1000000);
}