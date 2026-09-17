#include <rom/ets_sys.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include <graphics.h>
#include <fonts.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "input_output.h"

typedef enum AppState {
    INSTRUCTIONS,
    PLAY,
    SCORE
} AppState;

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
AppState currentState = INSTRUCTIONS;
int currentScore = 0;

void reset_game() {
    //reset game values
    currentState = PLAY;
    setFontColour(255, 255, 0);
    player->pos.y = 50;
    currentScore = 0;

    //clear all enemies
    EnemyNode *current = enemies->headPointer;
    while (current != NULL) {
        pop_node(enemies);
        current = enemies->headPointer;
    }

    //add new node
    add_node(enemies);
}

void update() {
    KEY_TYPE key = getInput();

    if (key != NO_INPUT) currentKey = key;

    if (currentState == INSTRUCTIONS) {
        if (currentKey == LEFT_DOWN) reset_game();
        return;
    } else if (currentState == SCORE) {
        vTaskDelay(pdMS_TO_TICKS(4000));
        reset_game();
        return;
    }

    //update player values based on current input
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
    if (player->pos.y <= 1 || player->pos.y + player->size.y >= 135) {
        player->pos.y += -(player->velocity.y);
        //temp, check timer works
        setFontColour(255, 255, 255);
        currentState = SCORE;
        return;
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

    //then if we can continue, add to score then pop and add a new node
    if (enemies->headPointer->enemy->pos.x + enemies->headPointer->enemy->size.x >= 240) {
        currentScore += 100;

        pop_node(enemies);
        add_node(enemies);
    }
}

void render() {
    cls(0);
    
    if (currentState != PLAY) {
        if (currentState == INSTRUCTIONS) {
            print_xy("Instructions:\n", 0, 0);
            print_xy("Bottom for down, top for up,\n", CENTER, (display_height >> 1) - 24);
            print_xy("avoid the enemies, get points.\n", CENTER, (display_height >> 1) - 8);
            print_xy("Press the left button to start\n", CENTER, (display_height >> 1) + 8);
        } else {
            char buffer[16];
            snprintf(buffer, sizeof(buffer), "Score: %i\n", currentScore);
            print_xy("You Died!\n", CENTER, (display_height >> 1) - 16);
            print_xy(buffer, CENTER, (display_height >> 1));
        }
        
        flip_frame();
        return;
    }

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

    //render score
    draw_rectangle(0, 0, 80, 16, rgbToColour(0,0,0));
    gprintf("Score: %i\n", currentScore);

    flip_frame();
}


void app_main() {
    graphics_init();
    setFont(FONT_UBUNTU16);
    //setFont(FONT_SMALL);

    input_output_init();

    //seed rand() with the current time
    srand(time(NULL)); 

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
}