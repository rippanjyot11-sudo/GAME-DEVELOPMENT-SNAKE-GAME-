#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;
const int CELL_SIZE = 20;

struct Point {
    int x, y;
};

enum Direction { UP, DOWN, LEFT, RIGHT };

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! " << Mix_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    Mix_Chunk* eatSound = Mix_LoadWAV("eat.wav");
    if (!eatSound) {
        std::cout << "Failed to load sound: " << Mix_GetError() << std::endl;
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        Mix_FreeChunk(eatSound);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        Mix_FreeChunk(eatSound);
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    srand(static_cast<unsigned int>(time(nullptr)));

    // Initial snake setup
    std::vector<Point> snake = { {300, 300}, {280, 300}, {260, 300} };
    Direction dir = RIGHT;

    // Initial food position
    Point food = { (rand() % (WINDOW_WIDTH / CELL_SIZE)) * CELL_SIZE,
                   (rand() % (WINDOW_HEIGHT / CELL_SIZE)) * CELL_SIZE };

    bool running = true;
    int speed = 150;  // milliseconds delay
    Uint32 lastMoveTime = 0;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        if (dir != DOWN) dir = UP;
                        break;
                    case SDLK_DOWN:
                        if (dir != UP) dir = DOWN;
                        break;
                    case SDLK_LEFT:
                        if (dir != RIGHT) dir = LEFT;
                        break;
                    case SDLK_RIGHT:
                        if (dir != LEFT) dir = RIGHT;
                        break;
                }
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastMoveTime > (Uint32)speed) {
            lastMoveTime = currentTime;

            // Move snake body
            for (int i = (int)snake.size() - 1; i > 0; --i)
                snake[i] = snake[i - 1];

            // Move snake head
            switch (dir) {
                case UP:    snake[0].y -= CELL_SIZE; break;
                case DOWN:  snake[0].y += CELL_SIZE; break;
                case LEFT:  snake[0].x -= CELL_SIZE; break;
                case RIGHT: snake[0].x += CELL_SIZE; break;
            }

            // Check wall collision - end game
            if (snake[0].x < 0 || snake[0].x >= WINDOW_WIDTH ||
                snake[0].y < 0 || snake[0].y >= WINDOW_HEIGHT) {
                running = false;
            }

            // Check self collision - end game
            for (size_t i = 1; i < snake.size(); ++i) {
                if (snake[i].x == snake[0].x && snake[i].y == snake[0].y) {
                    running = false;
                    break;
                }
            }

            // Check if food eaten
            if (snake[0].x == food.x && snake[0].y == food.y) {
                // Play eating sound
                Mix_PlayChannel(-1, eatSound, 0);

                // Add new segment at tail (same position as last segment for now)
                snake.push_back(snake.back());

                // Spawn new food
                bool foodOnSnake;
                do {
                    foodOnSnake = false;
                    food.x = (rand() % (WINDOW_WIDTH / CELL_SIZE)) * CELL_SIZE;
                    food.y = (rand() % (WINDOW_HEIGHT / CELL_SIZE)) * CELL_SIZE;
                    for (auto& segment : snake) {
                        if (segment.x == food.x && segment.y == food.y) {
                            foodOnSnake = true;
                            break;
                        }
                    }
                } while (foodOnSnake);

                // Increase speed but limit minimum delay
                if (speed > 40)
                    speed -= 5;
            }
        }

        // Rendering
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background
        SDL_RenderClear(renderer);

        // Draw food (red)
        SDL_Rect foodRect = { food.x, food.y, CELL_SIZE, CELL_SIZE };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &foodRect);

        // Draw snake (green)
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (auto& segment : snake) {
            SDL_Rect rect = { segment.x, segment.y, CELL_SIZE, CELL_SIZE };
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    Mix_FreeChunk(eatSound);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "Game Over! Your score: " << (snake.size() - 3) << std::endl;

    return 0;
}
