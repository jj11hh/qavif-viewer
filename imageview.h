#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include "Image.h"
#include <glad/glad.h>

class ImageView {
public:
    ImageView();
    ~ImageView();

    void LoadFromImage(const Image& img);
    void Draw(float zoom);
    
    GLuint GetTextureID() const { return textureID; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    bool IsLoaded() const { return textureID != 0; }

private:
    GLuint textureID = 0;
    int width = 0;
    int height = 0;
    
    void Destroy();
};

#endif // IMAGEVIEW_H