#include "mainwindow.h"
#include "myimagereader.h"
#include <SDL3/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <cstdio>

int main(int argc, char** argv) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create window with graphics context
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    SDL_Window* window = SDL_CreateWindow("Qavif Viewer", 1280, 720, window_flags);
    if (!window) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Create Renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == nullptr)
    {
        SDL_Log("Error creating SDL_Renderer!");
        return 0;
    }
    SDL_SetRenderVSync(renderer, 1);

    // Set Icon
    {
        MyImageReader iconReader("Images/icon.png");
        Image icon = iconReader.read();
        if (icon.valid) {
            SDL_Surface* surface = SDL_CreateSurfaceFrom(
                icon.width,
                icon.height,
                SDL_PIXELFORMAT_RGBA32,
                (void*)icon.data.data(),
                icon.width * 4
            );

            if (surface) {
                SDL_SetWindowIcon(window, surface);
                SDL_DestroySurface(surface);
            }
        }
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; 

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    MainWindow app;
    app.SetRenderer(renderer);

    if (argc > 1) {
        app.LoadFile(argv[1]);
    }

    bool done = false;
    while (!done) {
        SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    ImGui_ImplSDL3_ProcessEvent(&event);
                    if (event.type == SDL_EVENT_QUIT)
                        done = true;
                    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                        done = true;
                    
                    if (event.type == SDL_EVENT_PINCH_UPDATE) {
                        app.OnPinch(event.pinch.scale);
                    }
                }
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        app.Render();

        ImGui::Render();
        
        float scale = SDL_GetWindowDisplayScale(window);
        SDL_SetRenderScale(renderer, scale, scale);
        
        SDL_SetRenderDrawColor(renderer, 115, 140, 153, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}