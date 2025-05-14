#include <SDL.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

using namespace std;

struct Enemy {
    int row, col;
    bool active;
    Uint32 lastMoveTime;
    float moveDelay;

    Enemy() : row(0), col(0), active(false), lastMoveTime(0), moveDelay(200.0f) {}
};

struct Qbert {
    int row, col;
    int lives;
    Qbert() : row(0), col(0), lives(3) {}
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL_Init error: " << SDL_GetError() << endl;
        return -1;
    }
    if (TTF_Init() < 0) {
        cerr << "TTF_Init error: " << TTF_GetError() << endl;
        SDL_Quit();
        return -1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "SDL_mixer error: " << Mix_GetError() << endl;
        return -1;
    }
    Mix_Chunk* startSound = Mix_LoadWAV("assets/sounds/start.wav");
    if (!startSound)
    {
        cerr << "Failed to load start sound: " << Mix_GetError() << endl;
    }
    Mix_Chunk* moveSound = Mix_LoadWAV("assets/sounds/jump.wav");
    if (!moveSound)
    {
        cerr << "Failed to load move sound: " << Mix_GetError() << endl;
    }
    Mix_Chunk* loseLifeSound = Mix_LoadWAV("assets/sounds/lose_life.wav");
    if (!loseLifeSound)
    {
        cerr << "Failed to load lose life sound: " << Mix_GetError() << endl;
    }
    Mix_Chunk* winSound = Mix_LoadWAV("assets/sounds/byebye.wav");
    if (!winSound)
    {
        cerr << "Failed to load win sound: " << Mix_GetError() << endl;
    }

    SDL_Window* window = SDL_CreateWindow("Q*bert_ripoff",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "SDL_CreateWindow error: " << SDL_GetError() << endl;
        SDL_Quit();
        return -1;
    }


    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "SDL_CreateRenderer error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    TTF_Font* font = TTF_OpenFont("assets/fonts/PressStart2P-Regular.ttf", 20);
    if (!font)
    {
        cerr << "TTF_OpenFont error: " << TTF_GetError() << endl;
        return -1;
    }

    SDL_Texture* qbertTexture = IMG_LoadTexture(renderer, "assets/images/qbert.png");
    SDL_Texture* enemyTexture = IMG_LoadTexture(renderer, "assets/images/slime1.png");
    SDL_Texture* enemyTexture1 = IMG_LoadTexture(renderer, "assets/images/slime2.png");
    SDL_Texture* tileNormalTexture = IMG_LoadTexture(renderer, "assets/images/cube1.png");
    SDL_Texture* tileVisitedTexture = IMG_LoadTexture(renderer, "assets/images/cube2.png");
    SDL_Texture* backgroundTexture = IMG_LoadTexture(renderer, "assets/images/background.png");

    if (!qbertTexture || !enemyTexture || !backgroundTexture || !enemyTexture1 ||!tileNormalTexture || !tileVisitedTexture)
    {
        cerr << "Failed to load image: " << IMG_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }


    bool running = true;
    bool gameOver = false;
    SDL_Event event;

    Uint32 startTime = SDL_GetTicks();
    int timeLimit = 48000;


    const int pyramidSize = 7;
    const int tileSize = 50;
    const int startX = 400;
    const int startY = 100;

    Qbert qbert;

    vector<vector<bool>> visited(pyramidSize);
    for (int i = 0; i < pyramidSize; ++i)
        visited[i] = vector<bool>(i + 1, false);

    Uint32 lastMoveTime = SDL_GetTicks();
    float moveDelay = 500.0f;

    Enemy enemy1, enemy2;
    enemy1.moveDelay = 600.0f;
    enemy2.moveDelay = 500.0f;
    srand(time(0));


    Uint32 lastEnemy1Time = SDL_GetTicks();
    Uint32 enemy1SpawnInterval = 4000;

    Uint32 lastEnemy2Time = SDL_GetTicks();
    Uint32 enemy2SpawnInterval = 6000;

    SDL_Color white = {255, 255, 255};
    if (startSound) Mix_PlayChannel(-1, startSound, 0);

    SDL_Surface* readySurface = TTF_RenderText_Solid(font, "READY?", white);
    if(readySurface)
    {
        SDL_Texture* readyTexture = SDL_CreateTextureFromSurface(renderer, readySurface);
        SDL_Rect readyRect = {300, 270, readySurface->w, readySurface->h};

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, readyTexture, nullptr, &readyRect);
        SDL_RenderPresent(renderer);

        SDL_FreeSurface(readySurface);
        SDL_DestroyTexture(readyTexture);
    }
    SDL_Delay(2000);

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (!gameOver && event.type == SDL_KEYDOWN && (SDL_GetTicks() - lastMoveTime) >= moveDelay) {
                switch (event.key.keysym.sym) {
                    case SDLK_DOWN:
                    case SDLK_s:
                        if (qbert.row + 1 < pyramidSize && qbert.col + 1 <= qbert.row + 1) {
                            qbert.row++;
                            qbert.col++;
                            lastMoveTime = SDL_GetTicks();
                            if (moveSound) Mix_PlayChannel(-1, moveSound, 0);
                        }
                        break;

                    case SDLK_RIGHT:
                    case SDLK_d:
                        if (qbert.row + 1 < pyramidSize && qbert.col <= qbert.row + 1) {
                            qbert.row++;
                            lastMoveTime = SDL_GetTicks();
                            if (moveSound) Mix_PlayChannel(-1, moveSound, 0);
                        }
                        break;

                    case SDLK_LEFT:
                    case SDLK_a:
                        if (qbert.row > 0 && qbert.col > 0) {
                            qbert.row--;
                            qbert.col--;
                            lastMoveTime = SDL_GetTicks();
                            if (moveSound) Mix_PlayChannel(-1, moveSound, 0);
                        }
                        break;

                    case SDLK_UP:
                    case SDLK_w:
                        if (qbert.row > 0 && qbert.col <= qbert.row - 1) {
                            qbert.row--;
                            lastMoveTime = SDL_GetTicks();
                            if (moveSound) Mix_PlayChannel(-1, moveSound, 0);
                        }
                        break;
                }
            }
        }

        string livesText = "Lives: " + to_string(qbert.lives);

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - startTime >= timeLimit) {
        gameOver = true;
        }

        if (!enemy1.active && (currentTime - lastEnemy1Time) >= enemy1SpawnInterval) {
            enemy1.row = rand() % pyramidSize;
            enemy1.col = rand() % (enemy1.row + 1);
            enemy1.active = true;
            lastEnemy1Time = currentTime;
        }

        if (!enemy2.active && (currentTime - lastEnemy2Time) >= enemy2SpawnInterval) {
            enemy2.row = rand() % pyramidSize;
            enemy2.col = rand() % (enemy2.row + 1);
            enemy2.active = true;
            lastEnemy2Time = currentTime;
        }

        if (enemy1.active && (currentTime - enemy1.lastMoveTime) >= enemy1.moveDelay) {
            if (enemy1.row < pyramidSize - 1) {
                enemy1.row++;
                enemy1.col = min(enemy1.col + 1, enemy1.row);
            } else {
                enemy1.active = false;
            }
            enemy1.lastMoveTime = currentTime;
        }

        if (enemy2.active && (currentTime - enemy2.lastMoveTime) >= enemy2.moveDelay) {
            if (enemy2.row < pyramidSize - 1) {
                enemy2.row++;
                enemy2.col = min(enemy2.col + 1, enemy2.row);

                if (visited[enemy2.row][enemy2.col]) {
                    visited[enemy2.row][enemy2.col] = false;
                }
            } else {
                enemy2.active = false;
            }
            enemy2.lastMoveTime = currentTime;
        }

        if (enemy1.active && qbert.row == enemy1.row && qbert.col == enemy1.col && qbert.lives > 0) {
            qbert.lives--;
            enemy1.active = false;
            if (qbert.lives == 0) gameOver = true;
            if (loseLifeSound) Mix_PlayChannel(-1, loseLifeSound, 0);
        }

        if (enemy2.active && qbert.row == enemy2.row && qbert.col == enemy2.col) {
            enemy2.active = false;
        }

        visited[qbert.row][qbert.col] = true;

        bool gameWon = true;
        for(int i=0; i < pyramidSize; i++)
            for(int j=0; j <= i; j++)
        {
            if(!visited[i][j])
            {
                gameWon = false;
                break;
            }
            if(!gameWon) break;
        }

        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        for (int row = 0; row < pyramidSize; ++row) {
            for (int col = 0; col <= row; ++col) {
                int x = startX - row * tileSize / 2 + col * tileSize;
                int y = startY + row * tileSize;

                SDL_Rect tile = { x, y, tileSize, tileSize };

                if (visited[row][col])
                    SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 217, 46, 95, 0.95);
                    SDL_RenderFillRect(renderer, &tile);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    SDL_RenderDrawRect(renderer, &tile);

            }
        }
//vẽ nvc
        int q_x = startX - qbert.row * tileSize / 2 + qbert.col * tileSize;
        int q_y = startY + qbert.row * tileSize;
        SDL_Rect qbertRect = { q_x, q_y, tileSize, tileSize };
        SDL_RenderCopy(renderer, qbertTexture, NULL, &qbertRect);
//vẽ quái 1
        if (enemy1.active) {
            int enemy1_x = startX - enemy1.row * tileSize / 2 + enemy1.col * tileSize;
            int enemy1_y = startY + enemy1.row * tileSize;
            SDL_Rect enemyRect = { enemy1_x + 10, enemy1_y + 10, tileSize - 20, tileSize - 20 };
            SDL_RenderCopy(renderer, enemyTexture, NULL, &enemyRect);
        }
//vẽ quái 2
        if (enemy2.active) {
            int enemy2_x = startX - enemy2.row * tileSize / 2 + enemy2.col * tileSize;
            int enemy2_y = startY + enemy2.row * tileSize;
            SDL_Rect enemyRect = { enemy2_x + 10, enemy2_y + 10, tileSize - 20, tileSize - 20 };
            SDL_RenderCopy(renderer, enemyTexture1, NULL, &enemyRect);

        }

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, livesText.c_str(), white);
        if (textSurface) {
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
            SDL_Rect textRect = {20, 20, textSurface->w, textSurface->h}; // góc trên trái màn hình
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }

        int timeRemaining = max(0, (timeLimit - (int)(currentTime - startTime)) / 1000);
        string timerText = "Time: " + to_string(timeRemaining);
        SDL_Surface* timerSurface = TTF_RenderText_Solid(font, timerText.c_str(), white);
        if (timerSurface) {
        SDL_Texture* timerTexture = SDL_CreateTextureFromSurface(renderer, timerSurface);
        SDL_Rect timerRect = {20, 60, timerSurface->w, timerSurface->h};
        SDL_RenderCopy(renderer, timerTexture, nullptr, &timerRect);
        SDL_FreeSurface(timerSurface);
        SDL_DestroyTexture(timerTexture);
        }

        if (gameOver) {
            SDL_Surface* overSurface = TTF_RenderText_Solid(font, "GAME OVER", white);
            if (overSurface) {
                SDL_Texture* overTexture = SDL_CreateTextureFromSurface(renderer, overSurface);
                SDL_Rect overRect = {300, 270, overSurface->w, overSurface->h};
                SDL_RenderCopy(renderer, overTexture, nullptr, &overRect);
                SDL_FreeSurface(overSurface);
                SDL_DestroyTexture(overTexture);
            }
            SDL_Delay(200);
            running = false;
        }

        if (gameWon)
    {
    SDL_Surface* winSurface = TTF_RenderText_Solid(font, "You won~", white);
    if (winSurface)
    {
        SDL_Texture* winTexture = SDL_CreateTextureFromSurface(renderer, winSurface);
        SDL_Rect winRect = {300, 250, winSurface->w, winSurface->h};

        for (int i = 0; i < 4; ++i)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            if (i % 2 == 0) {
                SDL_RenderCopy(renderer, winTexture, nullptr, &winRect);
                if (winSound) Mix_PlayChannel(-1, winSound, 0);
            }

            SDL_RenderPresent(renderer);
            SDL_Delay(200);
        }

        SDL_FreeSurface(winSurface);
        SDL_DestroyTexture(winTexture);
    }
    running = false;
    }

        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    if (winSound) Mix_FreeChunk(winSound);
    if (moveSound) Mix_FreeChunk(moveSound);
    if (loseLifeSound) Mix_FreeChunk(loseLifeSound);
    if (startSound) Mix_FreeChunk(startSound);
    Mix_CloseAudio();
    SDL_DestroyTexture(qbertTexture);
    SDL_DestroyTexture(enemyTexture);
    SDL_DestroyTexture(enemyTexture1);
    SDL_DestroyTexture(tileNormalTexture);
    SDL_DestroyTexture(tileVisitedTexture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_Quit();
    return 0;
}
