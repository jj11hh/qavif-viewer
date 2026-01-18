# QAVIF Viewer

A lightweight, high-performance image viewer and converter for AVIF and JPEG images, built with **Dear ImGui** and **SDL3**.

![QAVIF Viewer Screenshot](Images/icon.png)

## Features

*   **View Images:** Fast loading and viewing of AVIF and JPEG images.
*   **Intuitive Zoom:** 
    *   **Mouse Wheel:** Pan (Scroll) naturally. Hold `Ctrl` to Zoom.
    *   **Touchpad:** Native Pinch-to-Zoom and Two-Finger Pan support.
    *   **Toolbar:** "Fit to Window", "1:1", and custom zoom slider.
*   **Conversion:** 
    *   Convert JPEG images to AVIF (with quality/speed settings).
    *   Convert AVIF images to JPEG.
    *   Preserve Exif metadata during conversion.
*   **Modern UI:** 
    *   Clean, dark-themed interface using Dear ImGui.
    *   **High DPI** support for crisp rendering on Retina/4K displays.
    *   Cross-platform file dialogs.

## Dependencies

*   [SDL3](https://libsdl.org/) (Windowing, Input, Graphics)
*   [Dear ImGui](https://github.com/ocornut/imgui) (User Interface)
*   [libavif](https://github.com/AOMediaCodec/libavif) (AVIF Decoding/Encoding)
*   [libjpeg-turbo](https://libjpeg-turbo.org/) (JPEG Decoding/Encoding)
*   [nativefiledialog-extended](https://github.com/btzy/nativefiledialog-extended) (System File Dialogs)
*   [vcpkg](https://github.com/microsoft/vcpkg) (Dependency Management)

## Build Instructions

This project uses **CMake** and **vcpkg** (manifest mode) for easy dependency management.

### Prerequisites

*   **C++ Compiler** (C++17 compliant)
*   **CMake** (>= 3.25)
*   **Git**

### Building

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/yourusername/qavif-viewer.git
    cd qavif-viewer
    ```

2.  **Bootstrap vcpkg:**
    If you haven't already, initialize the local vcpkg instance (or use your system-wide vcpkg):
    ```bash
    git submodule update --init --recursive
    ./vcpkg/bootstrap-vcpkg.sh  # (or .bat on Windows)
    ```

3.  **Configure with CMake:**
    This will automatically download and build all dependencies (SDL3, ImGui, etc.). This may take a few minutes the first time.
    ```bash
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
    ```

4.  **Build:**
    ```bash
    cmake --build build --config Release
    ```

5.  **Run:**
    ```bash
    ./build/qavif-viewer
    ```

## Controls

| Input | Action |
| :--- | :--- |
| **Left Click + Drag** | Pan image (if zoomed in) |
| **Scroll / 2-Finger Pan** | Pan image (Vertical/Horizontal) |
| **Ctrl + Scroll** | Zoom In / Out |
| **Pinch Gesture** | Zoom In / Out (Touchpad) |

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.