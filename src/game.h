#ifndef GAME_H
#define GAME_H

#include <stdlib.h>

typedef struct Vec2 {
    int x, y;
} Vec2;

typedef struct Entity {
    Vec2 size, pos, velocity;
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

Entity* create_entity(Vec2 S, Vec2 P, void* Sp);
void init_game();
void add_node(LinkedList *list);
void pop_node(LinkedList *list);
void reset_game();
int collision(Vec2 pos0, Vec2 size0, Vec2 pos1, Vec2 size1);

void change_speed_callback(void* arg);
void add_enemy_callback(void* arg);

#endif