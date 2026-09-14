#include "input_output.h"

#include <soc/uart_struct.h>
#include <soc/gpio_struct.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include <rom/ets_sys.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <stdio.h>

static QueueHandle_t inputQueue;
static TimerHandle_t repeatTimer;
static int buttonVal[2] = {1, 1};
int keyRepeat = 1;
static uint64_t lastKeyPress = 0;

const int leftButton = 0, rightButton = 35;

static void repeatTimerCallback(TimerHandle_t pxTimer) {
    int v;
    if(buttonVal[0] == 0) {
        v = leftButton;
        xQueueSendFromISR(inputQueue, &v, 0);
    }

    if(buttonVal[1] == 0) {
        v = rightButton;
        xQueueSendFromISR(inputQueue, &v, 0);
    }

    xTimerChangePeriod( repeatTimer, pdMS_TO_TICKS(200), 0);
    xTimerStart( repeatTimer, 0 );
}

// Interrupt Service Routine
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    uint64_t now = esp_timer_get_time();

    uint32_t gpioNum = (uint32_t) arg;
    int gpioIndex = (gpioNum == 35);
    int val = (1 - buttonVal[gpioIndex]);

    //ets_printf("gpio_isr_handler %d %d %lld\n", gpioNum, val, now - lastKeyPress);

    //ignore interrupts within 50 ms
    if (now - lastKeyPress > 50000) {
        // Send the button event to the queue
        //send base gpio num, and add 100 if the button is reading high
        int v = gpioNum + val * 100;
        xQueueSendFromISR(inputQueue, &v, NULL);
        
        //start/stop timer
        if(val == 0 && keyRepeat) {
            xTimerChangePeriodFromISR(repeatTimer, pdMS_TO_TICKS(400), 0);
            xTimerStartFromISR(repeatTimer, 0);
        }

        if(val == 1 && keyRepeat) {
            xTimerStopFromISR(repeatTimer, 0);
        }
        lastKeyPress = now;
    }
    buttonVal[gpioIndex] = val;

    gpio_set_intr_type(
        gpioNum, 
        val == 0 ?
            GPIO_INTR_HIGH_LEVEL
                :
            GPIO_INTR_LOW_LEVEL
    );
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
    
    // Create a queue for button events
    inputQueue = xQueueCreate(16, 4);
    repeatTimer = xTimerCreate(
        "repeat",
        pdMS_TO_TICKS(300),
        pdFALSE,
        (void*)0, 
        repeatTimerCallback
    );
    
    // Configure buttons as input
    gpio_set_direction(35, GPIO_MODE_INPUT);
    gpio_set_direction(0, GPIO_MODE_INPUT);
    // Interrupt when a button is pressed
    gpio_set_intr_type(35, GPIO_INTR_LOW_LEVEL);
    gpio_set_intr_type(0, GPIO_INTR_LOW_LEVEL);
    // Install the GPIO interrupt service
    gpio_install_isr_service(0);
    // Attach ISR to GPIO 35 and 0
    gpio_isr_handler_add(35, gpio_isr_handler, (void *)rightButton);
    gpio_isr_handler_add(0, gpio_isr_handler, (void *)leftButton);
}