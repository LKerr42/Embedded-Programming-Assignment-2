#include "input_output.h"

#include <soc/uart_struct.h>
#include <soc/gpio_struct.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include <rom/ets_sys.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "timer.h"
#include <stdio.h>

GPIOState *GPIOhandler;
extern timer* timers[4];

static QueueHandle_t inputQueue;
static int buttonVal[2] = {1, 1};
int keyRepeat = 1;
static uint64_t lastKeyPress = 0;

const int leftButton = 0, rightButton = 35;

static void repeatTimerCallback(void* arg) {
    int v;
    if(buttonVal[0] == 0) {
        v = leftButton;
        xQueueSend(inputQueue, &v, 0);
    }

    if(buttonVal[1] == 0) {
        v = rightButton;
        xQueueSend(inputQueue, &v, 0);
    }

    update_timer_period(timers[3], 0.2);
    start_timer(timers[3]);
}

static void gpio_isr_callback() {
    uint32_t gpioNum = *(int *)GPIOhandler->argument;

    //gpio_intr_disable(gpioNum);
    int gpioIndex = (gpioNum == 35);
    int val = gpio_get_level(gpioNum);

    //ets_printf("gpio_isr_handler %d %d %lld\n", gpioNum, val, now - lastKeyPress);

    // Send the button event to the queue
    //send base gpio num, and add 100 if the button is reading high
    int v = gpioNum + val * 100;
    xQueueSend(inputQueue, &v, 0);
    
    //start/stop timer
    if(val == 0 && keyRepeat) {
        update_timer_period(timers[3], 0.4);
        start_timer(timers[3]);
    }

    if(val == 1 && keyRepeat) {
        stop_timer(timers[3]);
    }

    buttonVal[gpioIndex] = val;

    //gpio_intr_enable(gpioNum);
}

// Interrupt Service Routine
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint64_t now = esp_timer_get_time();

    if (now - lastKeyPress > 50000) {
        GPIOhandler->triggered = 1;
        GPIOhandler->argument = arg;
        lastKeyPress = now;
    }
}

KEY_TYPE getInput() {
    int key;
    if(xQueueReceive(inputQueue, &key, 0) == pdFALSE) return NO_INPUT;
    
    switch(key) {
        case leftButton: return LEFT_DOWN;
        case rightButton: return RIGHT_DOWN;
        case 100 + leftButton: return LEFT_UP;
        case 100 + rightButton: return RIGHT_UP;
    }
    return NO_INPUT;
}

void input_output_init() {
    //ets_printf("INPUT OUTPUT INIT STARTED\n");
    //create handler
    GPIOhandler = calloc(1, sizeof(GPIOState));
    GPIOhandler->triggered = 0;
    GPIOhandler->callback = gpio_isr_callback;
    
    // Create a queue for button events
    inputQueue = xQueueCreate(16, 4);

    //create timer
    timers[3] = init_timer(
        0.3, 1, 1, repeatTimerCallback, NULL
    );
    
    // Configure buttons as input
    gpio_set_direction(35, GPIO_MODE_INPUT);
    gpio_set_direction(0, GPIO_MODE_INPUT);
    // Interrupt when a button is pressed
    gpio_set_intr_type(35, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(0, GPIO_INTR_ANYEDGE);
    // Install the GPIO interrupt service
    gpio_install_isr_service(0);
    // Attach ISR to GPIO 35 and 0
    gpio_isr_handler_add(35, gpio_isr_handler, &rightButton);
    gpio_isr_handler_add(0, gpio_isr_handler, &leftButton);
}