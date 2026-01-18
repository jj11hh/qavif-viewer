#include "mainwindow.h"
#include "myimagereader.h"
#include "jpegavifconverter.h"
#include "imgui.h"
#include "nfd.h"
#include <cstdio>
#include <iostream>

MainWindow::MainWindow() {
    // Initialize NFD
    NFD_Init();
}

MainWindow::~MainWindow() {
    NFD_Quit();
}

void MainWindow::OnPinch(float scale) {
    m_pendingPinchScale *= scale;
}

void MainWindow::Render() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    window_flags |= ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("MainDockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    DrawMenuBar();
    DrawToolBar();

    ImGui::BeginChild("ImageArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    // Handle Zoom Logic (Mouse Wheel + Pinch)
    ImGuiIO& io = ImGui::GetIO();
    float wheel = io.MouseWheel;
    
    // Zoom only with Pinch OR (Wheel + Ctrl)
    bool pinchZoom = (m_pendingPinchScale != 1.0f);
    bool wheelZoom = (wheel != 0.0f) && io.KeyCtrl;
    
    bool zooming = pinchZoom || wheelZoom;
    
    if (zooming && m_imageView.IsLoaded()) {
        float oldZoom = m_zoom;
        float newZoom = oldZoom;
        
        if (wheelZoom) {
            // Less aggressive zoom for wheel
            if (wheel > 0) newZoom *= 1.1f;
            else newZoom /= 1.1f;
        }
        
        if (pinchZoom) {
            // Soften the pinch? Raw scale might be delta?
            // SDL_EVENT_TOUCHPAD_PINCH gives 'scale' which is relative to previous? 
            // Or absolute for the gesture?
            // Usually it's a delta. 1.0 + delta.
            // Let's assume OnPinch passes the scale factor (e.g. 1.01 or 0.99).
            newZoom *= m_pendingPinchScale;
            m_pendingPinchScale = 1.0f;
        }

        // Clamp
        if (newZoom < 0.1f) newZoom = 0.1f;
        if (newZoom > 10.0f) newZoom = 10.0f; // Increase max zoom

        if (newZoom != oldZoom) {
            // Calculate center point in content coordinates
            // We want the point at the center of the viewport to remain at the center.
            // Or if mouse is hovering, the point under mouse.
            
            // ImGui Child Window content size is determined by the Image size drawn previously? 
            // Or we calculate it: ImageWidth * Zoom.
            
            ImVec2 viewportSize = ImGui::GetWindowSize();
            float scrollX = ImGui::GetScrollX();
            float scrollY = ImGui::GetScrollY();
            
            // Point relative to the top-left of the content
            // We use the center of the viewport as the anchor if wheel is used without mouse?
            // Or better: Use Mouse Pos if inside window, else Center.
            
            ImVec2 anchorPos; // In window/viewport space (0,0 is top-left of viewport)
            if (ImGui::IsWindowHovered()) {
                 ImVec2 mousePos = ImGui::GetMousePos();
                 ImVec2 winPos = ImGui::GetWindowPos();
                 anchorPos = ImVec2(mousePos.x - winPos.x, mousePos.y - winPos.y);
            } else {
                 anchorPos = ImVec2(viewportSize.x * 0.5f, viewportSize.y * 0.5f);
            }

            // Content Position of the anchor:
            // ContentX = (ScrollX + AnchorX) / oldZoom
            float contentX = (scrollX + anchorPos.x) / oldZoom;
            float contentY = (scrollY + anchorPos.y) / oldZoom;
            
            // Apply new zoom
            m_zoom = newZoom;
            
            // New Scroll Position:
            // NewScrollX = ContentX * newZoom - AnchorX
            float newScrollX = contentX * newZoom - anchorPos.x;
            float newScrollY = contentY * newZoom - anchorPos.y;
            
            ImGui::SetScrollX(newScrollX);
            ImGui::SetScrollY(newScrollY);
        }
    }
    
    m_imageView.Draw(m_zoom);
    ImGui::EndChild();

    DrawStatusBar();
    
    DrawSettingsModal();
    DrawAboutModal();

    ImGui::End();
}

void MainWindow::LoadFile(const std::string& path) {
    m_currentFilePath = path;
    m_statusMessage = "Loading " + m_currentFilePath;
    
    MyImageReader reader(m_currentFilePath);
    Image img = reader.read();
    if (img.valid) {
        if (m_renderer) {
            m_imageView.LoadFromImage(img, m_renderer);
            m_statusMessage = "Loaded " + m_currentFilePath;
        } else {
            m_statusMessage = "Renderer not set";
        }
    } else {
        m_statusMessage = "Failed to load " + m_currentFilePath;
    }
}

void MainWindow::DrawMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open")) {
                OpenFile();
            }
            if (ImGui::MenuItem("Save As...")) {
                SaveFile();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // Exit logic
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
             if (ImGui::MenuItem("Settings")) {
                 m_showSettings = true;
             }
             ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("About")) {
            if (ImGui::MenuItem("About Qavif")) {
                m_showAbout = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void MainWindow::DrawToolBar() {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);

    if (ImGui::Button("Open")) {
        OpenFile();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save As")) {
        SaveFile();
    }
    ImGui::SameLine();
    ImGui::Text("|");
    ImGui::SameLine();
    
    if (ImGui::Button("Fit Window")) {
        if (m_imageView.IsLoaded()) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float sW = (float)m_imageView.GetWidth();
            float sH = (float)m_imageView.GetHeight();
            float scale = 1.0f;
            if (sW > 0 && sH > 0) {
                float scaleX = avail.x / sW;
                float scaleY = avail.y / sH;
                scale = (scaleX < scaleY) ? scaleX : scaleY;
            }
            m_zoom = scale;
            if (m_zoom > 3.0f) m_zoom = 3.0f;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("1:1")) {
        m_zoom = 1.0f;
    }
    ImGui::SameLine();
    
    ImGui::SetNextItemWidth(150);
    ImGui::SliderFloat("Zoom", &m_zoom, 0.1f, 3.0f, "%.2fx");
    
    ImGui::Dummy(ImVec2(0, 4)); 
    ImGui::Separator();
}

void MainWindow::DrawStatusBar() {
    ImGui::Text(" %s", m_statusMessage.c_str());
    if (m_imageView.IsLoaded()) {
        ImGui::SameLine();
        ImGui::Text("| %d x %d", m_imageView.GetWidth(), m_imageView.GetHeight());
    }
}

void MainWindow::OpenFile() {
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog(&outPath, NULL, 0, NULL);
    
    if (result == NFD_OKAY) {
        LoadFile(outPath);
        NFD_FreePath(outPath);
    } else if (result == NFD_CANCEL) {
    } else {
        m_statusMessage = "Error opening file dialog";
    }
}

void MainWindow::SaveFile() {
    if (!m_imageView.IsLoaded()) return;

    nfdchar_t *outPath = NULL;
    nfdfilteritem_t filterItem[2] = { { "AVIF Image", "avif" }, { "JPEG Image", "jpg,jpeg" } };
    nfdresult_t result = NFD_SaveDialog(&outPath, filterItem, 2, NULL, NULL);

    if (result == NFD_OKAY) {
        std::string savePath = outPath;
        std::string ext = "";
        size_t dot = savePath.find_last_of(".");
        if (dot != std::string::npos) {
            ext = savePath.substr(dot + 1);
        } else {
            savePath += ".avif";
            ext = "avif";
        }
        
        for (auto &c : ext) c = tolower(c);

        JpegAvifConverter converter(m_settings);
        bool success = false;
        
        if (!m_currentFilePath.empty()) {
            std::string srcExt = "";
            size_t sdot = m_currentFilePath.find_last_of(".");
            if (sdot != std::string::npos) srcExt = m_currentFilePath.substr(sdot+1);
            for (auto &c : srcExt) c = tolower(c);

            if ((srcExt == "jpg" || srcExt == "jpeg") && ext == "avif") {
                success = converter.ConvertJpegToAvif(m_currentFilePath, savePath);
            }
            else if (srcExt == "avif" && (ext == "jpg" || ext == "jpeg")) {
                success = converter.ConvertAvifToJpeg(m_currentFilePath, savePath);
            }
            else {
                 MyImageReader reader(m_currentFilePath);
                 Image img = reader.read();
                 if (ext == "avif") {
                     success = converter.ImageToAvif(img, savePath);
                 } else {
                     m_statusMessage = "Saving to " + ext + " not fully supported from raw.";
                 }
            }
        }

        if (success) {
            m_statusMessage = "Saved to " + savePath;
        } else {
            m_statusMessage = "Failed to save " + savePath;
        }
        
        NFD_FreePath(outPath);
    }
}

void MainWindow::DrawSettingsModal() {
    if (m_showSettings) {
        ImGui::OpenPopup("Settings");
        m_showSettings = false;
    }

    if (ImGui::BeginPopupModal("Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::SeparatorText("AVIF Encoding");
        ImGui::SliderInt("Max Quantizer", &m_settings.maxQuantizer, 0, 63);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("0 is lossless, 63 is worst quality");
        ImGui::SliderInt("Min Quantizer", &m_settings.minQuantizer, 0, 63);
        ImGui::SliderInt("Speed", &m_settings.encodeSpeed, 0, 10);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("0 is slowest (best compression), 10 is fastest");
        ImGui::Checkbox("Keep AVIF EXIF", &m_settings.isSaveAvifExif);

        ImGui::SeparatorText("JPEG Encoding");
        ImGui::SliderInt("Quality", &m_settings.jpegQuality, 0, 100);
        ImGui::Checkbox("Keep JPEG EXIF", &m_settings.isSaveJpegExif);

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) { 
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) { 
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::EndPopup();
    }
}

void MainWindow::DrawAboutModal() {
    if (m_showAbout) {
        ImGui::OpenPopup("About Qavif");
        m_showAbout = false;
    }
    
    if (ImGui::BeginPopupModal("About Qavif", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Qavif Viewer (ImGui Port)");
        ImGui::Text("Original by: jiangyiheng"); 
        ImGui::Text("Refactored to remove Qt dependency.");
        ImGui::Separator();
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}