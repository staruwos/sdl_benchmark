#include <SDL.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <iostream>

const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 600;
const int RECT_SIZE = 20; 
const int INITIAL_COUNT = 500;

struct MovingRect {
    float x, y;
    float dx, dy;
    SDL_Color color;
};

float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

SDL_Color randomColor() {
    return { 
        static_cast<Uint8>(rand() % 256), 
        static_cast<Uint8>(rand() % 256), 
        static_cast<Uint8>(rand() % 256), 
        255 
    };
}

void addRects(std::vector<MovingRect>& rects, int count) {
    for (int i = 0; i < count; ++i) {
        rects.push_back({
            randomFloat(0, WINDOW_WIDTH - RECT_SIZE),
            randomFloat(0, WINDOW_HEIGHT - RECT_SIZE),
            randomFloat(-200.0f, 200.0f),
            randomFloat(-200.0f, 200.0f),
            randomColor()
        });
    }
}

int main(int argc, char* argv[]) {
#ifdef USE_GLES2
    // Force OpenGL ES 2.0 Driver
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");    
    // Disable batching (can sometimes cause the "flickering" issue)
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "0");
#endif

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create Window (Fullscreen Desktop is safest for Wayland)
    SDL_Window* window = SDL_CreateWindow("SDL2 Benchmark", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        WINDOW_WIDTH, WINDOW_HEIGHT, 
        SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_SHOWN);
    
    if (!window) return 1;

    // Create Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "Renderer Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // --- DEBUG: Print the active driver ---
    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    std::cout << "ACTIVE DRIVER: " << info.name << std::endl;
    std::cout << "TEXTURE FORMATS: " << info.num_texture_formats << std::endl;
    // --------------------------------------

    // Force Logical Size
    // This fixes the "Giant Square" by forcing the projection matrix to 1024x600
    SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

    std::vector<MovingRect> rects;
    srand(static_cast<unsigned int>(time(0)));
    addRects(rects, INITIAL_COUNT);

    bool quit = false;
    SDL_Event e;
    Uint32 lastTime = SDL_GetTicks();
    int frameCount = 0;
    Uint32 fpsTimer = lastTime;

    while (!quit) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) quit = true;
                if (e.key.keysym.sym == SDLK_UP) addRects(rects, 1000);
                if (e.key.keysym.sym == SDLK_DOWN && rects.size() > 1000) rects.resize(rects.size() - 1000);
            }
        }

        for (auto& rect : rects) {
            rect.x += rect.dx * deltaTime;
            rect.y += rect.dy * deltaTime;

            if (rect.x < 0 || rect.x > WINDOW_WIDTH - RECT_SIZE) rect.dx = -rect.dx;
            if (rect.y < 0 || rect.y > WINDOW_HEIGHT - RECT_SIZE) rect.dy = -rect.dy;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (const auto& rect : rects) {
            SDL_SetRenderDrawColor(renderer, rect.color.r, rect.color.g, rect.color.b, rect.color.a);
            SDL_Rect r = { static_cast<int>(rect.x), static_cast<int>(rect.y), RECT_SIZE, RECT_SIZE };
            SDL_RenderFillRect(renderer, &r);
        }

        SDL_RenderPresent(renderer);

        frameCount++;
        if (currentTime - fpsTimer >= 1000) {
            std::cout << "FPS: " << frameCount << " | Objs: " << rects.size() << std::endl;
            frameCount = 0;
            fpsTimer = currentTime;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
