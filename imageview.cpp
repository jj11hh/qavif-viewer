#include "imageview.h"
#include "imgui.h"

ImageView::ImageView() {}

ImageView::~ImageView() {
    Destroy();
}

void ImageView::Destroy() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

void ImageView::LoadFromImage(const Image& img) {
    Destroy();
    if (!img.valid) return;

    width = img.width;
    height = img.height;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear for zoom
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data.data());
}

void ImageView::Draw(float zoom) {
    if (!textureID) return;

    ImVec2 size((float)width * zoom, (float)height * zoom);
    
    // Center the image in the available space
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float x = (avail.x - size.x) * 0.5f;
    float y = (avail.y - size.y) * 0.5f;
    
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + x, ImGui::GetCursorPosY() + y));
    ImGui::Image((ImTextureID)(intptr_t)textureID, size);
}