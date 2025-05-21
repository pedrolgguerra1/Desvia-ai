#include<stdio.h>
#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 20
#define ENEMY_SIZE 20
#define MAX_ENEMIES 10

typedef struct {
    Vector2 position;
    bool active;
} Enemy;

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Desvia ai");
    SetTargetFPS(60);
    srand(time(NULL));

}