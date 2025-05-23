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

typedef enum { STATE_MENU, STATE_PLAYING, STATE_PAUSED, STATE_GAMEOVER } GameState;

void resetGame(Enemy enemies[], Vector2 bulletPos[], bool bulletActive[], Rectangle *player, int *lives, int *score, bool *gameOver) {
    *player = (Rectangle){ SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - 50, PLAYER_WIDTH, PLAYER_HEIGHT };
    *lives = 3;
    *score = 0;
    *gameOver = false;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].position = (Vector2){ rand() % (SCREEN_WIDTH - ENEMY_SIZE), -(rand() % 600) };
        enemies[i].active = true;
        enemies[i].hits = 0;
    }
    for (int i = 0; i < MAX_BULLETS; i++) {
        bulletActive[i] = false;
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Desvia ai");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    srand(time(NULL));

    Sound shootSound = LoadSound("pew.mp3");
    Music music = LoadMusicStream("trilha-sonora.mp3");
    PlayMusicStream(music);
    float musicVolume = 1.0f;
    bool isMuted = false;

    Rectangle player;
    Enemy enemies[MAX_ENEMIES];
    Vector2 bulletPos[MAX_BULLETS];
    bool bulletActive[MAX_BULLETS];
    int lives, score;
    bool gameOver;
    bool damaged = false;
    float damageTimer = 0.0f;
    const float damageDuration = 0.3f;
    float playerSpeed = 5.0f;

    GameState state = STATE_MENU;
    float shootCooldownTimer = 0.0f;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateMusicStream(music);

        if (state == STATE_MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                resetGame(enemies, bulletPos, bulletActive, &player, &lives, &score, &gameOver);
                damaged = false;
                damageTimer = 0.0f;
                shootCooldownTimer = 0.0f;
                state = STATE_PLAYING;
            }
        }
        else if (state == STATE_PLAYING) {
            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_PAUSED;

            if (!gameOver) {
                if (shootCooldownTimer > 0.0f) {
                    shootCooldownTimer -= deltaTime;
                    if (shootCooldownTimer < 0.0f) shootCooldownTimer = 0.0f;
                }

                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.x -= playerSpeed;
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.x += playerSpeed;
                if (player.x < 0) player.x = 0;
                if (player.x > SCREEN_WIDTH - PLAYER_WIDTH) player.x = SCREEN_WIDTH - PLAYER_WIDTH;

                if (IsKeyDown(KEY_SPACE) && shootCooldownTimer <= 0.0f) {
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bulletActive[i]) {
                            bulletPos[i] = (Vector2){ player.x + PLAYER_WIDTH / 2 - 2, player.y };
                            bulletActive[i] = true;
                            PlaySound(shootSound);
                            shootCooldownTimer = SHOOT_COOLDOWN / 60.0f;
                            break;
                        }
                    }
                }

                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bulletActive[i]) {
                        bulletPos[i].y -= BULLET_SPEED;
                        if (bulletPos[i].y < 0) bulletActive[i] = false;
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
                            damaged = true;
                            damageTimer = damageDuration;
                            enemies[i].position.y = -ENEMY_SIZE;
                            enemies[i].position.x = rand() % (SCREEN_WIDTH - ENEMY_SIZE);
                            enemies[i].hits = 0;
                            if (lives <= 0) gameOver = true;
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

                if (damaged) {
                    damageTimer -= deltaTime;
                    if (damageTimer <= 0.0f) damaged = false;
                }
            } else {
                state = STATE_GAMEOVER;
            }
        }
        else if (state == STATE_PAUSED) {
            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_PLAYING;

            Vector2 mouse = GetMousePosition();
            Rectangle btnMute = { SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 50, 240, 40 };
            if (CheckCollisionPointRec(mouse, btnMute)) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    isMuted = !isMuted;
                    SetMusicVolume(music, isMuted ? 0.0f : musicVolume);
                }
            }
        }
        else if (state == STATE_GAMEOVER) {
            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_MENU;

            Vector2 mousePoint = GetMousePosition();
            Rectangle btnRec = { SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 60, 240, 40 };
            if (CheckCollisionPointRec(mousePoint, btnRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                resetGame(enemies, bulletPos, bulletActive, &player, &lives, &score, &gameOver);
                damaged = false;
                damageTimer = 0.0f;
                shootCooldownTimer = 0.0f;
                state = STATE_PLAYING;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (state == STATE_MENU) {
            DrawText("DESVIA AI", SCREEN_WIDTH/2 - MeasureText("DESVIA AI", 50)/2, 200, 50, YELLOW);
            DrawText("Press ENTER to start", SCREEN_WIDTH/2 - MeasureText("Press ENTER to start", 20)/2, 300, 20, LIGHTGRAY);
        }
        else if (state == STATE_PLAYING) {
            DrawRectangleRec(player, damaged ? RED : BLUE);
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    Color color = (enemies[i].hits == 1) ? ORANGE : RED;
                    DrawRectangleV(enemies[i].position, (Vector2){ENEMY_SIZE, ENEMY_SIZE}, color);
                }
            }
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bulletActive[i]) {
                    DrawRectangle(bulletPos[i].x, bulletPos[i].y, 4, 10, YELLOW);
                }
            }
            DrawText(TextFormat("Pontos: %d", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Vidas: %d", lives), SCREEN_WIDTH - 100, 10, 20, WHITE);
        }
        else if (state == STATE_PAUSED) {
            DrawText("PAUSE", SCREEN_WIDTH/2 - MeasureText("PAUSE", 40)/2, 200, 40, YELLOW);
            Rectangle btnMute = { SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 50, 240, 40 };
            DrawRectangleRec(btnMute, DARKGRAY);
            const char *label = isMuted ? "Desmutar Música" : "Mutar Música";
            DrawText(label, btnMute.x + (btnMute.width - MeasureText(label, 20)) / 2, btnMute.y + 10, 20, WHITE);
        }
        else if (state == STATE_GAMEOVER) {
            DrawText("GAME OVER", SCREEN_WIDTH/2 - MeasureText("GAME OVER", 40)/2, 200, 40, RED);
            char finalScore[64];
            sprintf(finalScore, "Pontuação Final: %d", score);
            DrawText(finalScore, SCREEN_WIDTH/2 - MeasureText(finalScore, 20)/2, 260, 20, WHITE);

            Rectangle btnRec = { SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 60, 240, 40 };
            DrawRectangleRec(btnRec, DARKGRAY);
            DrawText("Jogar Novamente", btnRec.x + (btnRec.width - MeasureText("Jogar Novamente", 20)) / 2, btnRec.y + 10, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    UnloadSound(shootSound);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
