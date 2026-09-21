#include "game.h"

GameState gameState;

extern void* spaceship_pointer;
extern void* asteroid_pointer;

static float currentEnemyVelocity = 2;
static int numberEnemies = 0;

void init_game() {
    //init player
    gameState.player = create_entity((Vec2F){11, 21}, (Vec2F){210, 50}, spaceship_pointer);

    //init enemies
    gameState.enemies = calloc(1, sizeof(LinkedList));
    add_node(gameState.enemies);
}

Entity* create_entity(Vec2F S, Vec2F P, void* Sp) {
    // Allocate memory
    Entity* e = calloc(1, sizeof(Entity)); 
    if (e == NULL) return NULL;     
    
    e->size = S;
    e->pos = P;
    e->sprite = Sp;
    
    return e; 
}

void add_node(LinkedList *list) {
    numberEnemies++;

    EnemyNode *temp = calloc(1, sizeof(EnemyNode));
    temp->enemy = create_entity((Vec2F){19, 9}, (Vec2F){0, rand() % 125}, asteroid_pointer);
    
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