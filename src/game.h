#ifndef GAME_H
#define GAME_H

#include <stdlib.h>

typedef struct Vec2F {
    float x, y;
} Vec2F;

typedef struct Entity {
    Vec2F size, pos, velocity;
    void* sprite;
} Entity;

typedef struct EnemyNode EnemyNode;

typedef struct EnemyNode {
    EnemyNode *fowards, *backwards;
    Entity *enemy;
} EnemyNode;

typedef struct LinkedList {
    EnemyNode *headPointer, *tailPointer;
} LinkedList;

typedef struct GameState {
    Entity *player;
    LinkedList *enemies;
    int score, highScore;
} GameState;

Entity* create_entity(Vec2F S, Vec2F P, void* Sp);
void init_game();
void add_node(LinkedList *list);
void pop_node(LinkedList *list);
void reset_game();

void change_speed_callback(void* arg);
void add_enemy_callback(void* arg);

#endif