#include <stdio.h>
#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define LARGURA_TELA 800
#define ALTURA_TELA 600
#define LARGURA_JOGADOR 50
#define ALTURA_JOGADOR 20
#define TAMANHO_INIMIGO 20
#define MAX_INIMIGOS 10
#define MAX_TIROS 20
#define VELOCIDADE_TIRO 7.0f
#define INTERVALO_TIRO 20

typedef struct {
    Vector2 position;
    bool active;
    int hits;
} Enemy;

typedef enum { STATE_MENU, STATE_PLAYING, STATE_PAUSED, STATE_GAMEOVER } GameState;

void resetGame(Enemy enemies[], Vector2 bulletPos[], bool bulletActive[], Rectangle *player, int *lives, int *score, bool *gameOver) {
    *player = (Rectangle){ LARGURA_TELA / 2 - LARGURA_JOGADOR / 2, ALTURA_TELA - 50, LARGURA_JOGADOR, ALTURA_JOGADOR };
    *lives = 3;
    *score = 0;
    *gameOver = false;

    for (int i = 0; i < MAX_INIMIGOS; i++) {
        enemies[i].position = (Vector2){ rand() % (LARGURA_TELA - TAMANHO_INIMIGO), -(rand() % 600) };
        enemies[i].active = true;
        enemies[i].hits = 0;
    }
    for (int i = 0; i < MAX_TIROS; i++) {
        bulletActive[i] = false;
    }
}

int main(void) {
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Desvia ai");
    InitAudioDevice();
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);
    srand(time(NULL));

    Sound shootSound = LoadSound("pew.mp3");
    Music music = LoadMusicStream("trilha-sonora.mp3");
    PlayMusicStream(music);
    float musicVolume = 1.0f;
    bool isMuted = false;

    Texture2D textureNave = LoadTexture("nave.png");
    Texture2D textureAsteroide = LoadTexture("asteroide.png");

    Rectangle player;
    Enemy enemies[MAX_INIMIGOS];
    Vector2 bulletPos[MAX_TIROS];
    bool bulletActive[MAX_TIROS];
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
                if (player.x > LARGURA_TELA - LARGURA_JOGADOR) player.x = LARGURA_TELA - LARGURA_JOGADOR;

                if (IsKeyDown(KEY_SPACE) && shootCooldownTimer <= 0.0f) {
                    for (int i = 0; i < MAX_TIROS; i++) {
                        if (!bulletActive[i]) {
                            bulletPos[i] = (Vector2){ player.x + LARGURA_JOGADOR / 2 - 2, player.y };
                            bulletActive[i] = true;
                            PlaySound(shootSound);
                            shootCooldownTimer = INTERVALO_TIRO / 60.0f;
                            break;
                        }
                    }
                }

                for (int i = 0; i < MAX_TIROS; i++) {
                    if (bulletActive[i]) {
                        bulletPos[i].y -= VELOCIDADE_TIRO;
                        if (bulletPos[i].y < 0) bulletActive[i] = false;
                    }
                }

                for (int i = 0; i < MAX_INIMIGOS; i++) {
                    if (enemies[i].active) {
                        enemies[i].position.y += 2;
                        if (enemies[i].position.y > ALTURA_TELA) {
                            enemies[i].position.y = -TAMANHO_INIMIGO;
                            enemies[i].position.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
                            enemies[i].hits = 0;
                        }

                        Rectangle enemyRec = { enemies[i].position.x, enemies[i].position.y, TAMANHO_INIMIGO, TAMANHO_INIMIGO };
                        if (CheckCollisionRecs(player, enemyRec)) {
                            lives--;
                            damaged = true;
                            damageTimer = damageDuration;
                            enemies[i].position.y = -TAMANHO_INIMIGO;
                            enemies[i].position.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
                            enemies[i].hits = 0;
                            if (lives <= 0) gameOver = true;
                        }
                    }
                }

                for (int i = 0; i < MAX_TIROS; i++) {
                    if (bulletActive[i]) {
                        for (int j = 0; j < MAX_INIMIGOS; j++) {
                            if (enemies[j].active) {
                                Rectangle bulletRec = { bulletPos[i].x, bulletPos[i].y, 4, 10 };
                                Rectangle enemyRec = { enemies[j].position.x, enemies[j].position.y, TAMANHO_INIMIGO, TAMANHO_INIMIGO };
                                if (CheckCollisionRecs(bulletRec, enemyRec)) {
                                    bulletActive[i] = false;
                                    enemies[j].hits++;
                                    if (enemies[j].hits >= 2) {
                                        enemies[j].active = false;
                                        score += 10;
                                        enemies[j].position.y = -TAMANHO_INIMIGO;
                                        enemies[j].position.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
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
            Rectangle btnMute = { LARGURA_TELA / 2 - 120, ALTURA_TELA / 2 + 50, 240, 40 };
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
            Rectangle btnRec = { LARGURA_TELA / 2 - 120, ALTURA_TELA / 2 + 60, 240, 40 };
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
            DrawText("DESVIA AI", LARGURA_TELA/2 - MeasureText("DESVIA AI", 50)/2, 200, 50, YELLOW);
            DrawText("Press ENTER to start", LARGURA_TELA/2 - MeasureText("Press ENTER to start", 20)/2, 300, 20, LIGHTGRAY);
        }
        else if (state == STATE_PLAYING) {
            DrawTextureEx(textureNave, (Vector2){ player.x, player.y }, 0.0f, 0.2f, damaged ? RED : WHITE);

            for (int i = 0; i < MAX_INIMIGOS; i++) {
                if (enemies[i].active) {
                    DrawTextureEx(textureAsteroide, enemies[i].position, 0.0f, 0.1f, enemies[i].hits == 1 ? GRAY : WHITE);
                }
            }

            for (int i = 0; i < MAX_TIROS; i++) {
                if (bulletActive[i]) {
                    DrawRectangle(bulletPos[i].x, bulletPos[i].y, 4, 10, YELLOW);
                }
            }
            DrawText(TextFormat("Pontos: %d", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Vidas: %d", lives), LARGURA_TELA - 100, 10, 20, WHITE);
        }
        else if (state == STATE_PAUSED) {
            DrawText("PAUSE", LARGURA_TELA/2 - MeasureText("PAUSE", 40)/2, 200, 40, YELLOW);
            Rectangle btnMute = { LARGURA_TELA / 2 - 120, ALTURA_TELA / 2 + 50, 240, 40 };
            DrawRectangleRec(btnMute, DARKGRAY);
            const char *label = isMuted ? "Desmutar Música" : "Mutar Música";
            DrawText(label, btnMute.x + (btnMute.width - MeasureText(label, 20)) / 2, btnMute.y + 10, 20, WHITE);
        }
        else if (state == STATE_GAMEOVER) {
            DrawText("GAME OVER", LARGURA_TELA/2 - MeasureText("GAME OVER", 40)/2, 200, 40, RED);
            char finalScore[64];
            sprintf(finalScore, "Pontuação Final: %d", score);
            DrawText(finalScore, LARGURA_TELA/2 - MeasureText(finalScore, 20)/2, 260, 20, WHITE);

            Rectangle btnRec = { LARGURA_TELA / 2 - 120, ALTURA_TELA / 2 + 60, 240, 40 };
            DrawRectangleRec(btnRec, DARKGRAY);
            DrawText("Jogar Novamente", btnRec.x + (btnRec.width - MeasureText("Jogar Novamente", 20)) / 2, btnRec.y + 10, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    UnloadTexture(textureNave);
    UnloadTexture(textureAsteroide);
    UnloadSound(shootSound);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
