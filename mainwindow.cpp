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
        m_imageView.LoadFromImage(img);
        m_statusMessage = "Loaded " + m_currentFilePath;
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
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
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
    
    if (result == NFD_OK) {
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

    if (result == NFD_OK) {
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
