#include <stdio.h>
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

    Rectangle player = { SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - 50, PLAYER_WIDTH, PLAYER_HEIGHT };
    float playerSpeed = 5.0f;

    Enemy enemies[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].position = (Vector2){ rand() % (SCREEN_WIDTH - ENEMY_SIZE), -(rand() % 600) };
        enemies[i].active = true;
    }

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_LEFT)) player.x -= playerSpeed;
        if (IsKeyDown(KEY_RIGHT)) player.x += playerSpeed;

        if (player.x < 0) player.x = 0;
        if (player.x > SCREEN_WIDTH - PLAYER_WIDTH) player.x = SCREEN_WIDTH - PLAYER_WIDTH;

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                enemies[i].position.y += 2;
                if (enemies[i].position.y > SCREEN_HEIGHT) {
                    enemies[i].position.y = -ENEMY_SIZE;
                    enemies[i].position.x = rand() % (SCREEN_WIDTH - ENEMY_SIZE);
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangleRec(player, BLUE);
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                DrawRectangleV(enemies[i].position, (Vector2){ENEMY_SIZE, ENEMY_SIZE}, RED);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}