#include <SDL3/SDL.h>
#include <cstdio>

int main() {
    SDL_Event e;
    e.type = SDL_EVENT_PINCH_UPDATE;
    float s = e.pinch.scale;
    return 0;
}
