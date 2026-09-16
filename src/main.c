#include <rom/ets_sys.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <soc/gpio_struct.h>
#include <driver/gpio.h>

#include <graphics.h>
#include <fonts.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "input_output.h"

typedef struct Colour {
    int r, g, b;
} Colour;

static const Colour WHITE  = {255, 255, 255};
// static const Colour BLACK  = {0, 0, 0};
static const Colour RED    = {255, 0, 0};
// static const Colour BLUE   = {0, 0, 255};
// static const Colour YELLOW = {255, 255, 0};
// static const Colour GREEN  = {0, 255, 0};
// static const Colour ORANGE = {255, 128, 0};
// static const Colour PURPLE = {191, 0, 255};

typedef struct Vec2 {
    int x, y;
} Vec2;

typedef struct Entity {
    Vec2 size, pos, velocity;
    Colour colour;
    uint8_t moving;
} Entity;

typedef struct EnemyNode EnemyNode;

typedef struct EnemyNode {
    EnemyNode *fowards, *backwards;
    Entity *enemy;
} EnemyNode;

typedef struct LinkedList {
    EnemyNode *headPointer, *tailPointer;
} LinkedList;

Entity* create_entity(Vec2 S, Vec2 P, Colour C) {
    // Allocate memory
    Entity* e = calloc(1, sizeof(Entity)); 
    if (e == NULL) return NULL;     
    
    e->size = S;
    e->pos = P;
    e->colour = C;
    
    return e; 
}

void add_node(LinkedList *list) {
    EnemyNode *temp = calloc(1, sizeof(EnemyNode));
    temp->enemy = create_entity((Vec2){20, 10}, (Vec2){0, rand() % 100}, WHITE);
    temp->enemy->moving = 1;
    temp->enemy->velocity = (Vec2){2, 0};

    temp->backwards = list->tailPointer;
    temp->fowards = NULL;

    if (list->headPointer == NULL) list->headPointer = temp;
    if (list->tailPointer != NULL) list->tailPointer->fowards = temp;
    list->tailPointer = temp;
}

void pop_node(LinkedList *list) {
    if (list->headPointer == NULL) return;

    EnemyNode *oldHead = list->headPointer;
    list->headPointer = oldHead->fowards;

    if (list->headPointer != NULL) {
        list->headPointer->backwards = NULL;
    } else {
        list->tailPointer = NULL;
    }

    //free memory
    free(oldHead->enemy);   
    free(oldHead);
}

Entity *player = NULL;
LinkedList *enemies = NULL;
KEY_TYPE currentKey = NO_INPUT;

void update() {
    KEY_TYPE key = getInput();

    if (key != NO_INPUT) currentKey = key;

    //update player values based on current inout
    switch (currentKey) {
        case LEFT_DOWN:
            player->velocity.y = 2;
            player->moving = 1;
            break;
        case RIGHT_DOWN:
            player->velocity.y = -2;
            player->moving = 1;
            break;   
        case LEFT_UP:
            player->velocity.y = 0;
            player->moving = 0;
            break;
        case RIGHT_UP:
            player->velocity.y = 0;
            player->moving = 0;
            break;
        case NO_INPUT:
            break; 
    }

    if (player->moving) {
        player->pos.x += player->velocity.x;
        player->pos.y += player->velocity.y;
    }

    //keep player in bounds
    if (player->pos.y <= 1 || player->pos.y >= 100) {
        player->pos.y += -(player->velocity.y);
    }

    //update all enemies
    EnemyNode *current = enemies->headPointer;
    while (current != NULL) {
        current->enemy->pos.x += current->enemy->velocity.x;
        current->enemy->pos.y += current->enemy->velocity.y;
        current = current->fowards;
    }

    //check for collision and 
    //first, confirm there is a front node
    if (enemies->headPointer == NULL) return;

    //then if we can continue, pop and add a new node
    if (enemies->headPointer->enemy->pos.x >= 210) {
        pop_node(enemies);
        add_node(enemies);
    }
}

void render() {
    cls(0);
    
    //draw player
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

    //draw all enemies
    EnemyNode *current = enemies->headPointer;
    while (current != NULL) {
        draw_rectangle(
            current->enemy->pos.x, 
            current->enemy->pos.y, 
            current->enemy->size.x, 
            current->enemy->size.y, 
            rgbToColour(
                current->enemy->colour.r,
                current->enemy->colour.g,
                current->enemy->colour.b
            )
        );

        current = current->fowards;
    }

    flip_frame();
}


void app_main() {
    graphics_init();
    setFont(FONT_DEJAVU18);

    input_output_init();

    //seed rand() with the current time
    srand(time(NULL)); 

    
    

    //wait for user input to start
    while (!gpio_get_level(0)) {
        //display instructions
        cls(0);
        gprintf("Me when\n");
        flip_frame();
    }

    //init player
    player = create_entity((Vec2){10, 20}, (Vec2){210, 50}, RED);

    //init enemies
    enemies = calloc(1, sizeof(LinkedList));
    add_node(enemies);

    while (1) {
        //ets_printf("GAME LOOP RUNNING\n");

        update();
        render();

        //vTaskDelay(pdMS_TO_TICKS(1000));
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