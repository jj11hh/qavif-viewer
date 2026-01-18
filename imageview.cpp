#include "imageview.h"
#include "imgui.h"

ImageView::ImageView() {}

ImageView::~ImageView() {
    Destroy();
}

void ImageView::Destroy() {
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }
}

void ImageView::LoadFromImage(const Image& img, SDL_Renderer* renderer) {
    Destroy();
    if (!img.valid || !renderer) return;

    width = img.width;
    height = img.height;

    // Create texture from surface or directly
    // Image data is RGBA8888
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;
#else
    Uint32 rmask = 0x000000ff;
    Uint32 gmask = 0x0000ff00;
    Uint32 bmask = 0x00ff0000;
    Uint32 amask = 0xff000000;
#endif

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        width,
        height,
        SDL_PIXELFORMAT_RGBA32,
        (void*)img.data.data(),
        width * 4
    );

    if (surface) {
        m_texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }
}

void ImageView::Draw(float zoom) {
    if (!m_texture) return;

    ImVec2 size((float)width * zoom, (float)height * zoom);
    
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float x = (avail.x - size.x) * 0.5f;
    float y = (avail.y - size.y) * 0.5f;
    
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + x, ImGui::GetCursorPosY() + y));
    ImGui::Image((ImTextureID)m_texture, size);
}