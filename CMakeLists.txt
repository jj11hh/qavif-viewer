cmake_minimum_required(VERSION 3.20)

project(qavif-viewer LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(libavif CONFIG REQUIRED)
find_package(libjpeg-turbo CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(SDL3 CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)

# STB does not always provide a config in vcpkg depending on version, 
# but usually it is header only. vcpkg installs it to include path.
find_path(STB_INCLUDE_DIR stb_image.h)

set(PROJECT_SOURCES
    main.cpp
    convertsettings.cpp
    convertsettings.h
    imageview.cpp
    imageview.h
    jpegavifconverter.cpp
    jpegavifconverter.h
    jpegheaderreader.cpp
    jpegheaderreader.h
    mainwindow.cpp
    mainwindow.h
    myimagereader.cpp
    myimagereader.h
)

add_executable(qavif-viewer ${PROJECT_SOURCES})

target_link_libraries(qavif-viewer PRIVATE
    avif
    $<IF:$<TARGET_EXISTS:libjpeg-turbo::turbojpeg>,libjpeg-turbo::turbojpeg,libjpeg-turbo::turbojpeg-static>
    imgui::imgui
    SDL3::SDL3
    nfd::nfd
)

target_include_directories(qavif-viewer PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${STB_INCLUDE_DIR}
)

if(WIN32)
    target_link_libraries(qavif-viewer PRIVATE ws2_32)
endif()

install(TARGETS qavif-viewer DESTINATION bin)