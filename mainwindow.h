#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "imageview.h"
#include "convertsettings.h"
#include <SDL3/SDL.h>
#include <string>

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    void SetRenderer(SDL_Renderer* renderer) { m_renderer = renderer; }
    void Render();
    void LoadFile(const std::string& path);
    void OnPinch(float scale);
    
private:
    ImageView m_imageView;
    ConvertSettings m_settings;
    SDL_Renderer* m_renderer = nullptr;
    std::string m_currentFilePath;
    float m_zoom = 1.0f;
    float m_pendingPinchScale = 1.0f;
    
    bool m_showSettings = false;
    bool m_showAbout = false;
    std::string m_statusMessage;

    void OpenFile();
    void SaveFile();
    
    void DrawMenuBar();
    void DrawToolBar();
    void DrawSettingsModal();
    void DrawAboutModal();
    void DrawStatusBar();
};

#endif // MAINWINDOW_H