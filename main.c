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
#define MAX_BULLETS 20
#define BULLET_SPEED 7.0f
#define SHOOT_COOLDOWN 20 

typedef struct {
    Vector2 position;
    bool active;
    int hits;
} Enemy;

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Desvia ai");
    SetTargetFPS(60);
    srand(time(NULL));

    Rectangle player = { SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - 50, PLAYER_WIDTH, PLAYER_HEIGHT };
    float playerSpeed = 5.0f;
    int lives = 3;
    int score = 0;
    bool gameOver = false;

    Enemy enemies[MAX_ENEMIES];
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].position = (Vector2){ rand() % (SCREEN_WIDTH - ENEMY_SIZE), -(rand() % 600) };
        enemies[i].active = true;
        enemies[i].hits = 0;
    }

    Vector2 bulletPos[MAX_BULLETS];
    bool bulletActive[MAX_BULLETS];
    for (int i = 0; i < MAX_BULLETS; i++) {
        bulletActive[i] = false;
    }
    
    int shootTimer = 0; 

    while (!WindowShouldClose() && !gameOver) {
        if (IsKeyDown(KEY_LEFT)) player.x -= playerSpeed;
        if (IsKeyDown(KEY_RIGHT)) player.x += playerSpeed;

        if (player.x < 0) player.x = 0;
        if (player.x > SCREEN_WIDTH - PLAYER_WIDTH) player.x = SCREEN_WIDTH - PLAYER_WIDTH;

        shootTimer++;
        if (shootTimer >= SHOOT_COOLDOWN) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!bulletActive[i]) {
                    bulletPos[i] = (Vector2){ player.x + PLAYER_WIDTH/2 - 2, player.y };
                    bulletActive[i] = true;
                    shootTimer = 0;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bulletActive[i]) {
                bulletPos[i].y -= BULLET_SPEED;
                
                if (bulletPos[i].y < 0) {
                    bulletActive[i] = false;
                }
            }
        }

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                enemies[i].position.y += 2;
                
                if (enemies[i].position.y > SCREEN_HEIGHT) {
                    enemies[i].position.y = -ENEMY_SIZE;
                    enemies[i].position.x = rand() % (SCREEN_WIDTH - ENEMY_SIZE);
                    enemies[i].hits = 0;
                }
                
                Rectangle enemyRec = { enemies[i].position.x, enemies[i].position.y, ENEMY_SIZE, ENEMY_SIZE };
                if (CheckCollisionRecs(player, enemyRec)) {
                    lives--;
                    enemies[i].position.y = -ENEMY_SIZE;
                    enemies[i].position.x = rand() % (SCREEN_WIDTH - ENEMY_SIZE);
                    enemies[i].hits = 0;
                    
                    if (lives <= 0) {
                        gameOver = true;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bulletActive[i]) {
                for (int j = 0; j < MAX_ENEMIES; j++) {
                    if (enemies[j].active) {
                        Rectangle bulletRec = { bulletPos[i].x, bulletPos[i].y, 4, 10 };
                        Rectangle enemyRec = { enemies[j].position.x, enemies[j].position.y, ENEMY_SIZE, ENEMY_SIZE };
                        
                        if (CheckCollisionRecs(bulletRec, enemyRec)) {
                            bulletActive[i] = false;
                            enemies[j].hits++;
                            
                            if (enemies[j].hits >= 2) {
                                enemies[j].active = false;
                                score += 10;
                                
                                enemies[j].position.y = -ENEMY_SIZE;
                                enemies[j].position.x = rand() % (SCREEN_WIDTH - ENEMY_SIZE);
                                enemies[j].active = true;
                                enemies[j].hits = 0;
                            }
                            break;
                        }
                    }
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangleRec(player, BLUE);
        
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                Color enemyColor = RED;
                if (enemies[i].hits == 1) {
                    enemyColor = ORANGE;
                }
                DrawRectangleV(enemies[i].position, (Vector2){ENEMY_SIZE, ENEMY_SIZE}, enemyColor);
            }
        }
        
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bulletActive[i]) {
                DrawRectangle(bulletPos[i].x, bulletPos[i].y, 4, 10, YELLOW);
            }
        }
        
        DrawText(TextFormat("Pontos: %d", score), 10, 10, 20, WHITE);
        DrawText(TextFormat("Vidas: %d", lives), SCREEN_WIDTH - 100, 10, 20, WHITE);
        
        if (gameOver) {
            DrawText("GAME OVER", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 30, 40, RED);
            DrawText(TextFormat("Pontuação Final: %d", score), SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 20, 20, WHITE);
        }

        EndDrawing();
    }


    CloseWindow();
}