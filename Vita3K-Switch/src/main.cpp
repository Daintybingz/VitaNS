#include <SDL2/SDL.h>

int main(int argc, char **argv) {
    // EARLY CRASH DEBUGGING: Write a file as soon as main starts
    FILE* early_log = fopen("sdmc:/switch/vitans/early_log.txt", "w");
    if (early_log) {
        fprintf(early_log, "main() started\n");
        fclose(early_log);
    } else {
        FILE* root_log = fopen("sdmc:/early_log.txt", "w");
        if (root_log) {
            fprintf(root_log, "main() started (root)\n");
            fclose(root_log);
        }
    }

    // Initialize SDL2 and create a window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return 1;
    }
    SDL_Window* win = SDL_CreateWindow("VitaNS Minimal UI", 100, 100, 640, 480, SDL_WINDOW_SHOWN);
    if (!win) {
        SDL_Quit();
        return 1;
    }

    // Main loop: show window for 2 seconds
    SDL_Delay(2000);

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
