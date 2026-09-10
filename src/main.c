#include <soc/uart_struct.h>
#include <soc/gpio_struct.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <graphics.h>
#include <fonts.h>
#include <stdio.h>

typedef struct Colour {
    int r, g, b;
} Colour;

typedef struct Vec2 {
    int x, y;
} Vec2;

typedef struct Player {
    Vec2 size, pos;
    Colour colour;
} Player;

QueueHandle_t inputQueue;
const int leftButton = 0, rightButton = 35;
Player *player = NULL;

// static const Colour WHITE  = {255, 255, 255};
// static const Colour BLACK  = {0, 0, 0};
static const Colour RED    = {255, 0, 0};
// static const Colour BLUE   = {0, 0, 255};
// static const Colour YELLOW = {255, 255, 0};
// static const Colour GREEN  = {0, 255, 0};
// static const Colour ORANGE = {255, 128, 0};
// static const Colour PURPLE = {191, 0, 255};

Player* create_player(Vec2 S, Vec2 P, Colour C) {
    // Allocate memory
    Player* p = calloc(1, sizeof(Player)); 
    if (p == NULL) return NULL;     
    
    p->size = S;
    p->pos = P;
    p->colour = C;
    
    return p; 
}

void do_a_task(char *msg) {
    cls(0);
    print_xy(msg, CENTER, CENTER);
    flip_frame();
    vTaskDelay(pdMS_TO_TICKS(200));
}

// Interrupt Service Routine
static void IRAM_ATTR gpio_isr_handler(void *arg) {
    static uint64_t last_time = 0;
    uint64_t now = esp_timer_get_time();

    //ignore interrupts within 50 ms
    if (now - last_time > 50000) {
        int event = *((int *)arg);
        // Send the button event to the queue
        xQueueSendFromISR(inputQueue, &event, NULL);
        last_time = now;
    }
}

void update() {
    int event;
    if (xQueueReceive(inputQueue, &event, 0) == pdTRUE) {
        if (event == leftButton) {
            player->pos.x -= 2;
        } else if (event == rightButton) {
            player->pos.x += 2;
        }
    }
}

void render() {
    cls(0);
    
    draw_rectangle(
        player->pos.x, 
        player->pos.y, 
        player->size.x, 
        player->size.y, 
        rgbToColour(
            player->colour.r,
            player->colour.g,
            player->colour.b
        )
    );

    flip_frame();
}


void app_main() {
    graphics_init();
    setFont(FONT_DEJAVU18);
    // Create a queue for button events
    inputQueue = xQueueCreate(1, sizeof(int));
    // Configure buttons as input
    gpio_set_direction(35, GPIO_MODE_INPUT);
    gpio_set_direction(0, GPIO_MODE_INPUT);
    // Interrupt when a button is pressed
    gpio_set_intr_type(35, GPIO_INTR_NEGEDGE);
    gpio_set_intr_type(0, GPIO_INTR_NEGEDGE);
    // Install the GPIO interrupt service
    gpio_install_isr_service(0);
    // Attach ISR to GPIO 35 and 0
    gpio_isr_handler_add(35, gpio_isr_handler, (void *)&rightButton);
    gpio_isr_handler_add(0, gpio_isr_handler, (void *)&leftButton);

    //init player
    player = create_player((Vec2){50, 25}, (Vec2){50, 50}, RED);

    while (1) {
        update();
        render();
    }

    // -- GRAPHICS DEMO --
    // graphics_init();
    // uint64_t current_time, last_time=esp_timer_get_time();

    // for(int i=0; i<135*240; i++) {
    //     frame_buffer[i]=i*2;
    // }
    // flip_frame();

    // while(gpio_get_level(0));

    // setFont(FONT_UBUNTU16);
    // while(1) {
    //     cls(0);
    //     for(int i=0; i<500; i++) {
    //         draw_line(
    //             rand() % display_width, 
    //             rand() % display_height,
    //             rand() % display_width,
    //             rand() % display_height,
    //             rand()
    //         );
    //     }

    //     current_time = esp_timer_get_time();
    //     draw_rectangle(0, 0, 80, 16, rgbToColour(30,30,100));
    //     gprintf("FPS:%.2f\n", 1.0e6f / (current_time - last_time));
    //     last_time = current_time;
        
    //     flip_frame();
    // }

    // -- BUTTON DEMO --
    // volatile unsigned *GPIO_OUTPUT_ENABLE=(unsigned *)0x3ff44020;
    // volatile unsigned *GPIO_OUTPUT=(unsigned *)0x3ff44004;
    // *GPIO_OUTPUT_ENABLE |= (1<<4);
    // while(1) {
    //     if(!(GPIO.in1.data & 8)) {
    //         // set gpio 4 to 1
    //         *GPIO_OUTPUT |= (1<<4);
    //     } else {
    //         // clear gpio 4
    //         *GPIO_OUTPUT &= ~(1<<4);
    //     }
    // }
}