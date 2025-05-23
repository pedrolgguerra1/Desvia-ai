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
    InitAudioDevice();
    Sound shootSound = LoadSound("pew.mp3");
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Desvia ai");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    srand(time(NULL));

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
            }
            else {
                state = STATE_GAMEOVER;
            }
        }
        else if (state == STATE_PAUSED) {
            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_PLAYING;
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
            const char *title = "DESVIA AI";
            int titleSize = 50;
            int titleWidth = MeasureText(title, titleSize);
            DrawText(title, (SCREEN_WIDTH - titleWidth) / 2, SCREEN_HEIGHT / 2 - 100, titleSize, YELLOW);

            const char *prompt = "Press ENTER to start";
            int promptSize = 20;
            int promptWidth = MeasureText(prompt, promptSize);
            DrawText(prompt, (SCREEN_WIDTH - promptWidth) / 2, SCREEN_HEIGHT / 2, promptSize, LIGHTGRAY);
        }
        else if (state == STATE_PLAYING) {
            DrawRectangleRec(player, damaged ? RED : BLUE);

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active) {
                    Color enemyColor = RED;
                    if (enemies[i].hits == 1) enemyColor = ORANGE;
                    DrawRectangleV(enemies[i].position, (Vector2){ ENEMY_SIZE, ENEMY_SIZE }, enemyColor);
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
            const char *pauseText = "PAUSE";
            int fontSize = 40;
            int textWidth = MeasureText(pauseText, fontSize);
            DrawText(pauseText, (SCREEN_WIDTH - textWidth) / 2, SCREEN_HEIGHT / 2 - 20, fontSize, YELLOW);
            const char *resumeText = "Press ESC to resume";
            int resumeSize = 20;
            int resumeWidth = MeasureText(resumeText, resumeSize);
            DrawText(resumeText, (SCREEN_WIDTH - resumeWidth) / 2, SCREEN_HEIGHT / 2 + 30, resumeSize, LIGHTGRAY);
        }
        else if (state == STATE_GAMEOVER) {
            const char *gameOverText = "GAME OVER";
            int gameOverFontSize = 40;
            int gameOverTextWidth = MeasureText(gameOverText, gameOverFontSize);
            DrawText(gameOverText, (SCREEN_WIDTH - gameOverTextWidth) / 2, SCREEN_HEIGHT / 2 - 30, gameOverFontSize, RED);

            char finalScoreText[64];
            snprintf(finalScoreText, sizeof(finalScoreText), "Pontuação Final: %d", score);
            int finalScoreFontSize = 20;
            int finalScoreTextWidth = MeasureText(finalScoreText, finalScoreFontSize);
            DrawText(finalScoreText, (SCREEN_WIDTH - finalScoreTextWidth) / 2, SCREEN_HEIGHT / 2 + 20, finalScoreFontSize, WHITE);

            Rectangle btnRec = { SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 60, 240, 40 };
            DrawRectangleRec(btnRec, DARKGRAY);

            const char *btnLabel = "Jogar Novamente";
            int btnFontSize = 20;
            int btnLabelWidth = MeasureText(btnLabel, btnFontSize);
            DrawText(btnLabel, btnRec.x + (btnRec.width - btnLabelWidth) / 2, btnRec.y + 10, btnFontSize, LIGHTGRAY);

            Vector2 mousePoint = GetMousePosition();
            if (CheckCollisionPointRec(mousePoint, btnRec)) {
                DrawRectangleRec(btnRec, GRAY);
            }
        }

        EndDrawing();
    }

    UnloadSound(shootSound);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
