#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include "Image.h"
#include <SDL3/SDL.h>

class ImageView {
public:
    ImageView();
    ~ImageView();

    void LoadFromImage(const Image& img, SDL_Renderer* renderer);
    void Draw(float zoom);
    
    SDL_Texture* GetTexture() const { return m_texture; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsLoaded() const { return m_texture != nullptr; }

private:
    SDL_Texture* m_texture = nullptr;
    int width = 0;
    int height = 0;
    
    void Destroy();
};

#endif // IMAGEVIEW_H