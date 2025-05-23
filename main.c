#include <stdio.h>
#include "raylib.h"
#include <stdlib.h>
#include <time.h>

#define LARGURA_TELA 800
#define ALTURA_TELA 600
#define LARGURA_JOGADOR 50
#define ALTURA_JOGADOR 20
#define TAMANHO_INIMIGO 40
#define MAX_INIMIGOS 10
#define MAX_TIROS 20
#define VELOCIDADE_TIRO 7.0f
#define INTERVALO_TIRO 0.3f
#define MAX_PONTUACOES_TOP 5

typedef struct NoInimigo {
    Vector2 posicao;
    bool ativo;
    int acertos;
    struct NoInimigo *proximo;
} NoInimigo;

typedef struct NoTiro {
    Vector2 posicao;
    bool ativo;
    struct NoTiro *proximo;
} NoTiro;

typedef enum { ESTADO_MENU, ESTADO_JOGANDO, ESTADO_PAUSADO, ESTADO_FIMDEJOGO } EstadoJogo;

int pontuacoesTop[MAX_PONTUACOES_TOP] = {0};

void carregarPontuacoesTop() {
    FILE *arquivo = fopen("top_score.txt", "r");
    if (arquivo) {
        for (int i = 0; i < MAX_PONTUACOES_TOP; i++) {
            fscanf(arquivo, "%d", &pontuacoesTop[i]);
        }
        fclose(arquivo);
    }
}

void salvarPontuacoesTop() {
    FILE *arquivo = fopen("top_score.txt", "w");
    if (arquivo) {
        for (int i = 0; i < MAX_PONTUACOES_TOP; i++) {
            fprintf(arquivo, "%d\n", pontuacoesTop[i]);
        }
        fclose(arquivo);
    }
}

void atualizarPontuacoesTop(int novaPontuacao) {
    for (int i = 0; i < MAX_PONTUACOES_TOP; i++) {
        if (novaPontuacao > pontuacoesTop[i]) {
            for (int j = MAX_PONTUACOES_TOP - 1; j > i; j--) {
                pontuacoesTop[j] = pontuacoesTop[j - 1];
            }
            pontuacoesTop[i] = novaPontuacao;
            break;
        }
    }
}

void adicionarInimigo(NoInimigo **cabeca, int x, int y) {
    NoInimigo *novoInimigo = malloc(sizeof(NoInimigo));
    novoInimigo->posicao = (Vector2){ x, y };
    novoInimigo->ativo = true;
    novoInimigo->acertos = 0;
    novoInimigo->proximo = *cabeca;
    *cabeca = novoInimigo;
}

void adicionarTiro(NoTiro **cabeca, Vector2 posicao) {
    NoTiro *novoTiro = malloc(sizeof(NoTiro));
    novoTiro->posicao = posicao;
    novoTiro->ativo = true;
    novoTiro->proximo = *cabeca;
    *cabeca = novoTiro;
}

void liberarInimigos(NoInimigo *cabeca) {
    while (cabeca) {
        NoInimigo *temp = cabeca;
        cabeca = cabeca->proximo;
        free(temp);
    }
}

void liberarTiros(NoTiro *cabeca) {
    while (cabeca) {
        NoTiro *temp = cabeca;
        cabeca = cabeca->proximo;
        free(temp);
    }
}

int main(void) {
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Desvia ai");
    SetExitKey(0);
    InitAudioDevice();
    SetTargetFPS(60);
    srand(time(NULL));

    Sound somTiro = LoadSound("pew.mp3");
    Music musica = LoadMusicStream("trilha-sonora.mp3");
    PlayMusicStream(musica);
    bool musicaSilenciada = false;

    Texture2D texturaNave = LoadTexture("nave.png");
    Texture2D texturaAsteroide = LoadTexture("asteroide.png");

    Rectangle jogador = { LARGURA_TELA/2 - LARGURA_JOGADOR/2, ALTURA_TELA - 60, LARGURA_JOGADOR, ALTURA_JOGADOR };
    int vidas = 3;
    int pontuacao = 0;
    bool danificado = false;
    float temporizadorDano = 0.0f, temporizadorRecargaTiro = 0.0f;
    const float duracaoDano = 0.3f;
    float velocidadeJogador = 5.0f;

    EstadoJogo estado = ESTADO_MENU;

    NoInimigo *inimigos = NULL;
    NoTiro *tiros = NULL;

    carregarPontuacoesTop();

    for (int i = 0; i < MAX_INIMIGOS; i++) {
        adicionarInimigo(&inimigos, rand() % (LARGURA_TELA - TAMANHO_INIMIGO), -(rand() % 600));
    }

    while (!WindowShouldClose()) {
        UpdateMusicStream(musica);

        if (IsKeyPressed(KEY_ESCAPE)) {
            if (estado == ESTADO_JOGANDO) {
                estado = ESTADO_PAUSADO;
            } else if (estado == ESTADO_PAUSADO) {
                estado = ESTADO_JOGANDO;
            }
        }

        float deltaTime = GetFrameTime();

        if (estado == ESTADO_MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                liberarInimigos(inimigos);
                liberarTiros(tiros);
                inimigos = NULL;
                tiros = NULL;
                for (int i = 0; i < MAX_INIMIGOS; i++) {
                    adicionarInimigo(&inimigos, rand() % (LARGURA_TELA - TAMANHO_INIMIGO), -(rand() % 600));
                }
                jogador.x = LARGURA_TELA/2 - LARGURA_JOGADOR/2;
                vidas = 3;
                pontuacao = 0;
                danificado = false;
                temporizadorDano = 0.0f;
                temporizadorRecargaTiro = 0.0f;
                estado = ESTADO_JOGANDO;
            }
        } 
        else if (estado == ESTADO_JOGANDO) {
            if (temporizadorRecargaTiro > 0.0f) temporizadorRecargaTiro -= deltaTime;

            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) jogador.x -= velocidadeJogador;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) jogador.x += velocidadeJogador;
            if (jogador.x < 0) jogador.x = 0;
            if (jogador.x > LARGURA_TELA - LARGURA_JOGADOR) jogador.x = LARGURA_TELA - LARGURA_JOGADOR;

            if (IsKeyDown(KEY_SPACE) && temporizadorRecargaTiro <= 0.0f) {
                Vector2 pos = { jogador.x + LARGURA_JOGADOR/2 - 2, jogador.y };
                adicionarTiro(&tiros, pos);
                PlaySound(somTiro);
                temporizadorRecargaTiro = INTERVALO_TIRO;
            }

            NoTiro *t = tiros;
            while (t) {
                if (t->ativo) {
                    t->posicao.y -= VELOCIDADE_TIRO;
                    if (t->posicao.y < 0) t->ativo = false;
                }
                t = t->proximo;
            }

            NoInimigo *i = inimigos;
            while (i) {
                if (i->ativo) {
                    i->posicao.y += 2;
                    if (i->posicao.y > ALTURA_TELA) {
                        i->posicao.y = -TAMANHO_INIMIGO;
                        i->posicao.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
                        i->acertos = 0;
                        i->ativo = true;
                    }

                    Rectangle recInimigo = { i->posicao.x, i->posicao.y, TAMANHO_INIMIGO, TAMANHO_INIMIGO };
                    if (CheckCollisionRecs(jogador, recInimigo)) {
                        vidas--;
                        danificado = true;
                        temporizadorDano = duracaoDano;
                        i->posicao.y = -TAMANHO_INIMIGO;
                        i->posicao.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
                        i->acertos = 0;
                        i->ativo = true;
                        if (vidas <= 0) {
                            estado = ESTADO_FIMDEJOGO;
                            atualizarPontuacoesTop(pontuacao);
                            salvarPontuacoesTop();
                        }
                    }
                }
                i = i->proximo;
            }

            t = tiros;
            while (t) {
                if (t->ativo) {
                    i = inimigos;
                    while (i) {
                        if (i->ativo) {
                            Rectangle recTiro = { t->posicao.x, t->posicao.y, 4, 10 };
                            Rectangle recInimigo = { i->posicao.x, i->posicao.y, TAMANHO_INIMIGO, TAMANHO_INIMIGO };
                            if (CheckCollisionRecs(recTiro, recInimigo)) {
                                t->ativo = false;
                                i->acertos++;
                                if (i->acertos >= 2) {
                                    pontuacao += 10;
                                    i->posicao.y = -TAMANHO_INIMIGO;
                                    i->posicao.x = rand() % (LARGURA_TELA - TAMANHO_INIMIGO);
                                    i->acertos = 0;
                                    i->ativo = true;
                                }
                                break;
                            }
                        }
                        i = i->proximo;
                    }
                }
                t = t->proximo;
            }

            if (danificado) {
                temporizadorDano -= deltaTime;
                if (temporizadorDano <= 0.0f) danificado = false;
            }
        } 
        else if (estado == ESTADO_PAUSADO) {
            Rectangle botaoMutar = { LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 80, 200, 40 };
            if (CheckCollisionPointRec(GetMousePosition(), botaoMutar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                musicaSilenciada = !musicaSilenciada;
                SetMusicVolume(musica, musicaSilenciada ? 0.0f : 1.0f);
            }
        } 
        else if (estado == ESTADO_FIMDEJOGO) {
            if (IsKeyPressed(KEY_ENTER)) {
                liberarInimigos(inimigos);
                liberarTiros(tiros);
                inimigos = NULL;
                tiros = NULL;
                for (int k = 0; k < MAX_INIMIGOS; k++) {
                    adicionarInimigo(&inimigos, rand() % (LARGURA_TELA - TAMANHO_INIMIGO), -(rand() % 600));
                }
                jogador.x = LARGURA_TELA/2 - LARGURA_JOGADOR/2;
                vidas = 3;
                pontuacao = 0;
                danificado = false;
                temporizadorDano = 0.0f;
                temporizadorRecargaTiro = 0.0f;
                estado = ESTADO_JOGANDO;
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (estado == ESTADO_MENU) {
            DrawText("DESVIA AI", LARGURA_TELA/2 - MeasureText("DESVIA AI", 50)/2, 200, 50, YELLOW);
            DrawText("Pressione ENTER para começar", LARGURA_TELA/2 - MeasureText("Pressione ENTER para começar", 20)/2, 300, 20, LIGHTGRAY);
        } 
        else if (estado == ESTADO_JOGANDO) {
            DrawTextureEx(texturaNave, (Vector2){ jogador.x, jogador.y }, 0.0f, 0.2f, danificado ? RED : WHITE);

            NoInimigo *i = inimigos;
            while (i) {
                if (i->ativo)
                    DrawTextureEx(texturaAsteroide, i->posicao, 0.0f, 0.15f, i->acertos == 1 ? GRAY : WHITE);
                i = i->proximo;
            }

            NoTiro *t = tiros;
            while (t) {
                if (t->ativo)
                    DrawRectangle(t->posicao.x, t->posicao.y, 4, 10, YELLOW);
                t = t->proximo;
            }

            DrawText(TextFormat("Pontos: %d", pontuacao), 10, 10, 20, WHITE);
            DrawText(TextFormat("Vidas: %d", vidas), LARGURA_TELA - 100, 10, 20, WHITE);

            for (int k = 0; k < MAX_PONTUACOES_TOP; k++) {
                DrawText(TextFormat("Top %d: %d", k + 1, pontuacoesTop[k]), 10, 40 + 20 * k, 15, GRAY);
            }
        } 
        else if (estado == ESTADO_PAUSADO) {
            DrawText("PAUSADO", LARGURA_TELA/2 - MeasureText("PAUSADO", 40)/2, ALTURA_TELA/2 - 40, 40, YELLOW);
            DrawText("Pressione ESC para continuar", LARGURA_TELA/2 - MeasureText("Pressione ESC para continuar", 20)/2, ALTURA_TELA/2 + 10, 20, LIGHTGRAY);

            Rectangle botaoMutar = { LARGURA_TELA/2 - 100, ALTURA_TELA/2 + 80, 200, 40 };
            DrawRectangleRec(botaoMutar, LIGHTGRAY);
            const char *textoBotao = musicaSilenciada ? "Desmutar" : "Mutar";
            DrawText(textoBotao, botaoMutar.x + (botaoMutar.width - MeasureText(textoBotao, 20)) / 2, botaoMutar.y + 10, 20, WHITE);
        } 
        else if (estado == ESTADO_FIMDEJOGO) {
            DrawText("FIM DE JOGO", LARGURA_TELA/2 - MeasureText("FIM DE JOGO", 50)/2, 200, 50, RED);
            DrawText(TextFormat("Sua pontuação: %d", pontuacao), LARGURA_TELA/2 - MeasureText(TextFormat("Sua pontuação: %d", pontuacao), 20)/2, 280, 20, WHITE);
            DrawText("Pressione ENTER para reiniciar", LARGURA_TELA/2 - MeasureText("Pressione ENTER para reiniciar", 20)/2, 320, 20, LIGHTGRAY);
        }

        EndDrawing();
    }

    liberarInimigos(inimigos);
    liberarTiros(tiros);

    UnloadSound(somTiro);
    UnloadMusicStream(musica);
    UnloadTexture(texturaNave);
    UnloadTexture(texturaAsteroide);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
