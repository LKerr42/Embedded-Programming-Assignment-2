#include "game.h"

extern GameState gameState;

static const Colour WHITE  = {255, 255, 255};
// static const Colour BLACK  = {0, 0, 0};
static const Colour RED    = {255, 0, 0};
// static const Colour BLUE   = {0, 0, 255};
// static const Colour YELLOW = {255, 255, 0};
// static const Colour GREEN  = {0, 255, 0};
// static const Colour ORANGE = {255, 128, 0};
// static const Colour PURPLE = {191, 0, 255};

static float currentEnemyVelocity = 2;
static int numberEnemies = 0;

void init_game() {
    //init player
    gameState.player = create_entity((Vec2F){10, 20}, (Vec2F){210, 50}, RED);

    //init enemies
    gameState.enemies = calloc(1, sizeof(LinkedList));
    add_node(gameState.enemies);
}

Entity* create_entity(Vec2F S, Vec2F P, Colour C) {
    // Allocate memory
    Entity* e = calloc(1, sizeof(Entity)); 
    if (e == NULL) return NULL;     
    
    e->size = S;
    e->pos = P;
    e->colour = C;
    
    return e; 
}

void add_node(LinkedList *list) {
    numberEnemies++;

    EnemyNode *temp = calloc(1, sizeof(EnemyNode));
    temp->enemy = create_entity((Vec2F){20, 10}, (Vec2F){0, rand() % 125}, WHITE);
    temp->enemy->moving = 1;
    temp->enemy->velocity = (Vec2F){currentEnemyVelocity, 0};

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

void reset_game() {
    //reset game values
    gameState.player->pos.y = 50;
    gameState.score = 0;
    currentEnemyVelocity = 2;
    numberEnemies = 0;

    //clear all enemies
    EnemyNode *current = gameState.enemies->headPointer;
    while (current != NULL) {
        pop_node(gameState.enemies);
        current = gameState.enemies->headPointer;
    }

    //add new node
    add_node(gameState.enemies);
}

void change_speed_callback(void* arg) {
    float speedDelta = 0.5;

    if (currentEnemyVelocity + speedDelta == 6) return;

    currentEnemyVelocity += speedDelta;

    EnemyNode *current = gameState.enemies->headPointer;
    while (current != NULL) {
        Vec2F oldVel = current->enemy->velocity;
        current->enemy->velocity = (Vec2F){currentEnemyVelocity, oldVel.y};
        current = current->fowards;
    }
}

void add_enemy_callback(void* arg) {
    if (numberEnemies == 10) return;

    add_node(gameState.enemies);
}