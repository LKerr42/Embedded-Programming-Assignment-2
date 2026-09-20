#ifndef GAME_H
#define GAME_H

#include <stdlib.h>

typedef struct Colour {
    int r, g, b;
} Colour;

typedef struct Vec2 {
    int x, y;
} Vec2;

typedef struct Vec2F {
    float x, y;
} Vec2F;

typedef struct Entity {
    Vec2F size, pos, velocity;
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

typedef struct GameState {
    Entity *player;
    LinkedList *enemies;
    int score;
} GameState;

Entity* create_entity(Vec2F S, Vec2F P, Colour C);
void init_game();
void add_node(LinkedList *list);
void pop_node(LinkedList *list);
void reset_game();

void change_speed_callback(void* arg);
void add_enemy_callback(void* arg);

#endif