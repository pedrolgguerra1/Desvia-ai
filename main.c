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
#define MAX_TOP_SCORES 5

typedef struct EnemyNode {
    Vector2 position;
    bool active;
    int hits;
    struct EnemyNode *next;
} EnemyNode;

typedef struct BulletNode {
    Vector2 position;
    bool active;
    struct BulletNode *next;
} BulletNode;

typedef enum { STATE_MENU, STATE_PLAYING, STATE_PAUSED, STATE_GAMEOVER } GameState;

int topScores[MAX_TOP_SCORES] = {0};

void loadTopScores() {
    FILE *file = fopen("top_score.txt", "r");
    if (file) {
        for (int i = 0; i < MAX_TOP_SCORES; i++) {
            fscanf(file, "%d", &topScores[i]);
        }
        fclose(file);
    }
}

void saveTopScores() {
    FILE *file = fopen("top_score.txt", "w");
    if (file) {
        for (int i = 0; i < MAX_TOP_SCORES; i++) {
            fprintf(file, "%d\n", topScores[i]);
        }
        fclose(file);
    }
}

void updateTopScores(int newScore) {
    for (int i = 0; i < MAX_TOP_SCORES; i++) {
        if (newScore > topScores[i]) {
            for (int j = MAX_TOP_SCORES - 1; j > i; j--) {
                topScores[j] = topScores[j - 1];
            }
            topScores[i] = newScore;
            break;
        }
    }
}

void addEnemy(EnemyNode **head, int x, int y) {
    EnemyNode *newEnemy = malloc(sizeof(EnemyNode));
    newEnemy->position = (Vector2){ x, y };
    newEnemy->active = true;
    newEnemy->hits = 0;
    newEnemy->next = *head;
    *head = newEnemy;
}

void addBullet(BulletNode **head, Vector2 position) {
    BulletNode *newBullet = malloc(sizeof(BulletNode));
    newBullet->position = position;
    newBullet->active = true;
    newBullet->next = *head;
    *head = newBullet;
}

void freeEnemies(EnemyNode *head) {
    while (head) {
        EnemyNode *temp = head;
        head = head->next;
        free(temp);
    }
}

void freeBullets(BulletNode *head) {
    while (head) {
        BulletNode *temp = head;
        head = head->next;
        free(temp);
    }
}

int main(void) {
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Desvia ai");
    InitAudioDevice();
    SetTargetFPS(60);
    srand(time(NULL));

    Sound shootSound = LoadSound("pew.mp3");
    Music music = LoadMusicStream("trilha-sonora.mp3");
    PlayMusicStream(music);
    float musicVolume = 1.0f;
    bool isMuted = false;

    Texture2D textureNave = LoadTexture("nave.png");
    Texture2D textureAsteroide = LoadTexture("asteroide.png");

    Rectangle player = { LARGURA_TELA/2 - LARGURA_JOGADOR/2, ALTURA_TELA - 60, LARGURA_JOGADOR, ALTURA_JOGADOR };
    int lives = 3, score = 0;
    bool damaged = false, gameOver = false;
    float damageTimer = 0.0f, shootCooldownTimer = 0.0f;
    const float damageDuration = 0.3f;
    float playerSpeed = 5.0f;

    GameState state = STATE_MENU;

    EnemyNode *enemies = NULL;
    BulletNode *bullets = NULL;

    loadTopScores();

    for (int i = 0; i < MAX_INIMIGOS; i++) {
        addEnemy(&enemies, rand() % (LARGURA_TELA - TAMANHO_INIMIGO), -(rand() % 600));
    }

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateMusicStream(music);

        if (state == STATE_MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                freeEnemies(enemies);
                freeBullets(bullets);
                enemies = NULL;
                bullets = NULL;
                for (int i = 0; i < MAX_INIMIGOS; i++) {
                    addEnemy(&enemies, rand() % (LARGURA_TELA - TAMANHO_INIMIGO), -(rand() % 600));
                }
                player.x = LARGURA_TELA/2 - LARGURA_JOGADOR/2;
                lives = 3;
                score = 0;
                gameOver = false;
                damaged = false;
                damageTimer = 0.0f;
                shootCooldownTimer = 0.0f;
                state = STATE_PLAYING;
            }
        } else if (state == STATE_PLAYING) {
            if (IsKeyPressed(KEY_ESCAPE)) state = STATE_PAUSED;

            if (!gameOver) {
                if (shootCooldownTimer > 0.0f) shootCooldownTimer -= deltaTime;

                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.x -= playerSpeed;
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.x += playerSpeed;
                if (player.x < 0) player.x = 0;
                if (player.x > LARGURA_TELA - LARGURA_JOGADOR) player.x = LARGURA_TELA - LARGURA_JOGADOR;

                if (IsKeyDown(KEY_SPACE) && shootCooldownTimer <= 0.0f) {
                    Vector2 pos = { player.x + LARGURA_JOGADOR/2 - 2, player.y };
                    addBullet(&bullets, pos);
                    PlaySound(shootSound);
                    shootCooldownTimer = INTERVALO_TIRO / 60.0f;
                }

                BulletNode *b = bullets;
                while (b) {
                    if (b->active) {
                        b->position.y -= VELOCIDADE_TIRO;
                        if (b->position.y < 0) b->active = false;
                    }
                    b = b->next;
                }

                EnemyNode *e = enemies;
                while (e) {
                    if (e->active) {
                        e->position.y += 2;
                        if (e->position.y > ALTURA_TELA) {
                            e->position.y = -TAMANHO_INIMIGO;
                            e->position.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
                            e->hits = 0;
                        }

                        Rectangle enemyRec = { e->position.x, e->position.y, TAMANHO_INIMIGO, TAMANHO_INIMIGO };
                        if (CheckCollisionRecs(player, enemyRec)) {
                            lives--;
                            damaged = true;
                            damageTimer = damageDuration;
                            e->position.y = -TAMANHO_INIMIGO;
                            e->position.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
                            e->hits = 0;
                            if (lives <= 0) {
                                gameOver = true;
                                updateTopScores(score);
                                saveTopScores();
                            }
                        }
                    }
                    e = e->next;
                }

                b = bullets;
                while (b) {
                    if (b->active) {
                        e = enemies;
                        while (e) {
                            if (e->active) {
                                Rectangle bulletRec = { b->position.x, b->position.y, 4, 10 };
                                Rectangle enemyRec = { e->position.x, e->position.y, TAMANHO_INIMIGO, TAMANHO_INIMIGO };
                                if (CheckCollisionRecs(bulletRec, enemyRec)) {
                                    b->active = false;
                                    e->hits++;
                                    if (e->hits >= 2) {
                                        score += 10;
                                        e->position.y = -TAMANHO_INIMIGO;
                                        e->position.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
                                        e->hits = 0;
                                    }
                                    break;
                                }
                            }
                            e = e->next;
                        }
                    }
                    b = b->next;
                }

                if (damaged) {
                    damageTimer -= deltaTime;
                    if (damageTimer <= 0.0f) damaged = false;
                }
            } else {
                state = STATE_GAMEOVER;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (state == STATE_MENU) {
            DrawText("DESVIA AI", LARGURA_TELA/2 - MeasureText("DESVIA AI", 50)/2, 200, 50, YELLOW);
            DrawText("Press ENTER to start", LARGURA_TELA/2 - MeasureText("Press ENTER to start", 20)/2, 300, 20, LIGHTGRAY);
        } else if (state == STATE_PLAYING) {
            DrawTextureEx(textureNave, (Vector2){ player.x, player.y }, 0.0f, 0.2f, damaged ? RED : WHITE);

            EnemyNode *e = enemies;
            while (e) {
                if (e->active)
                    DrawTextureEx(textureAsteroide, e->position, 0.0f, 0.1f, e->hits == 1 ? GRAY : WHITE);
                e = e->next;
            }

            BulletNode *b = bullets;
            while (b) {
                if (b->active)
                    DrawRectangle(b->position.x, b->position.y, 4, 10, YELLOW);
                b = b->next;
            }

            DrawText(TextFormat("Pontos: %d", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("Vidas: %d", lives), LARGURA_TELA - 100, 10, 20, WHITE);

            for (int i = 0; i < MAX_TOP_SCORES; i++) {
                DrawText(TextFormat("Top %d: %d", i+1, topScores[i]), 10, 40 + i * 20, 18, GRAY);
            }
        }

        EndDrawing();
    }

    freeEnemies(enemies);
    freeBullets(bullets);
    UnloadTexture(textureNave);
    UnloadTexture(textureAsteroide);
    UnloadSound(shootSound);
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}