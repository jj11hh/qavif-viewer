# QAVIF Viewer

A simple and efficient image viewer and converter for AVIF images, built with Qt 6.

## Features

*   **View Images:** Open and view AVIF images (and other formats supported by Qt).
*   **Navigation:** Easily navigate through images in a folder (Next/Previous).
*   **Zoom Controls:** Zoom in/out, fit to window, and view at original size.
*   **Conversion:** 
    *   Convert JPEG images to AVIF.
    *   Convert AVIF images to JPEG.

## Dependencies

*   [Qt 6](https://www.qt.io/) (Core, Gui, Widgets, Concurrent, LinguistTools)
*   [libavif](https://github.com/AOMediaCodec/libavif)
*   [libjpeg-turbo](https://libjpeg-turbo.org/)

## Build Instructions

This project uses CMake and vcpkg for dependency management.

### Prerequisites

*   C++ Compiler (supporting C++17)
*   CMake (>= 3.20)
*   Qt 6
*   vcpkg

### Building

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/yourusername/qavif-viewer.git
    cd qavif-viewer
    ```

2.  **Configure with CMake (using vcpkg):**
    Ensure you have `vcpkg` installed and the toolchain file path is correct.

    ```bash
    # Example assuming vcpkg is in a sibling directory
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
    ```

3.  **Build:**
    ```bash
    cmake --build build --config Release
    ```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
