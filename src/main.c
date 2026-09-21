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
#include "game.h"
#include "timer.h"

typedef enum AppState {
    INSTRUCTIONS,
    PLAY,
    SCORE
} AppState;

GameState gameState;

KEY_TYPE currentKey = NO_INPUT;
AppState currentState = INSTRUCTIONS;

//abritrary timers, the fourth timer is reserved for io
timer* timers[4] = {NULL, NULL, NULL, NULL};

extern GPIOState *GPIOhandler;

extern image_header spaceship;

void set_app_state(AppState newState) {
    if (newState == PLAY) {
        reset_game();
        setFontColour(255, 255, 0);
    }
    currentState = newState;
}

void restart_game_callback(void* arg) {
    set_app_state(PLAY);

    stop_timer(timers[2]);
    reset_timer(timers[0]);
    reset_timer(timers[1]);
}

void update() {
    //handle timers
    for (int i = 0; i < 4; i++) {
        if (timers[i] == NULL) continue;
        if (timers[i]->triggered) {
            timers[i]->callback(timers[i]->argument);
            timers[i]->triggered = 0;
        }
    }

    //handle GPIO callback
    if (GPIOhandler->triggered) {
        GPIOhandler->callback();
        GPIOhandler->triggered = 0;
    }

    //handle inputs
    KEY_TYPE key = getInput();

    if (key != NO_INPUT) currentKey = key;

    //handle alt app states
    if (currentState == INSTRUCTIONS) {
        if (currentKey == LEFT_DOWN) {
            set_app_state(PLAY);
            start_timer(timers[0]);
            start_timer(timers[1]);
        }
        return;
    } else if (currentState == SCORE) {
        start_timer(timers[2]);
        return;
    }

    //update gameState.player values based on current input
    switch (currentKey) {
        case LEFT_DOWN:
            gameState.player->velocity.y = 2;
            gameState.player->moving = 1;
            break;
        case RIGHT_DOWN:
            gameState.player->velocity.y = -2;
            gameState.player->moving = 1;
            break;   
        case LEFT_UP:
            gameState.player->velocity.y = 0;
            gameState.player->moving = 0;
            break;
        case RIGHT_UP:
            gameState.player->velocity.y = 0;
            gameState.player->moving = 0;
            break;
        case NO_INPUT:
            break; 
    }

    if (gameState.player->moving) {
        gameState.player->pos.x += gameState.player->velocity.x;
        gameState.player->pos.y += gameState.player->velocity.y;
    }

    //keep gameState.player in bounds
    if (gameState.player->pos.y <= 1 || gameState.player->pos.y + gameState.player->size.y >= 135) {
        gameState.player->pos.y += -(gameState.player->velocity.y);
        //temp, check timer works
        // setFontColour(255, 255, 255);
        // set_app_state(SCORE);

        // if (gameState.score > gameState.highScore) gameState.highScore = gameState.score;
        // return;
    }

    //update all gameState.enemies
    EnemyNode *current = gameState.enemies->headPointer;
    while (current != NULL) {
        current->enemy->pos.x += current->enemy->velocity.x;
        current->enemy->pos.y += current->enemy->velocity.y;
        current = current->fowards;
    }

    //check for collision and enemy death
    //first, confirm there is a front node
    if (gameState.enemies->headPointer == NULL) return;

    //then if we can continue, add to score then pop and add a new node
    if (gameState.enemies->headPointer->enemy->pos.x + gameState.enemies->headPointer->enemy->size.x >= 240) {
        gameState.score += 100;

        pop_node(gameState.enemies);
        add_node(gameState.enemies);
    }
}

void render() {
    cls(0);
    
    //handle rendering for alt app states
    if (currentState != PLAY) {
        if (currentState == INSTRUCTIONS) {
            print_xy("Instructions:\n", 0, 0);
            print_xy("Bottom for down, top for up,\n", CENTER, (display_height >> 1) - 24);
            print_xy("avoid the enemies, get points.\n", CENTER, (display_height >> 1) - 8);
            print_xy("Press the left button to start\n", CENTER, (display_height >> 1) + 8);
        } else {
            char scoreBuffer[32], highScoreBuffer[32];
            snprintf(scoreBuffer, sizeof(scoreBuffer), "Score this game: %i\n", gameState.score);
            snprintf(highScoreBuffer, sizeof(highScoreBuffer), "Your high Score: %i\n", gameState.highScore);

            print_xy("You Died!\n", CENTER, (display_height >> 1) - 24);
            print_xy(scoreBuffer, CENTER, (display_height >> 1) - 8);
            print_xy(highScoreBuffer, CENTER, (display_height >> 1) + 8);
        }
        
        flip_frame();
        return;
    }

    //draw gameState.player
    // draw_rectangle(
    //     gameState.player->pos.x, 
    //     gameState.player->pos.y, 
    //     gameState.player->size.x, 
    //     gameState.player->size.y, 
    //     rgbToColour(
    //         gameState.player->colour.r,
    //         gameState.player->colour.g,
    //         gameState.player->colour.b
    //     )
    // );
    draw_image(
        (image_header *) &spaceship, 
        gameState.player->pos.x + 5, 
        gameState.player->pos.y + 10
    );
    //draw_pixel(gameState.player->pos.x, gameState.player->pos.y, rgbToColour(255, 0, 0));



    //draw all gameState.enemies
    EnemyNode *current = gameState.enemies->headPointer;
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
    //draw_rectangle(0, 0, 80, 16, rgbToColour(0,0,0));
    gprintf("Score: %i\n", gameState.score);

    flip_frame();
}


void app_main() {
    graphics_init();
    setFont(FONT_UBUNTU16);

    input_output_init();
    init_game();

    //seed rand() with the current time
    srand(time(NULL)); 

    //timers
    timers[0] = init_timer(40.0, 0, 0, change_speed_callback, NULL);
    timers[1] = init_timer(15.0, 0, 1, add_enemy_callback, NULL);
    timers[2] = init_timer(4.0, 1, 0, restart_game_callback, NULL);

    while (1) {
        update();
        render();
    }
}