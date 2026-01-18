cmake_minimum_required(VERSION 3.20)

project(qavif-viewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Concurrent LinguistTools)
find_package(libavif CONFIG REQUIRED)
find_package(libjpeg-turbo CONFIG REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PROJECT_SOURCES
    main.cpp
    convertsettings.cpp
    convertsettings.h
    dialogsettings.cpp
    dialogsettings.h
    dialogsettings.ui
    imageview.cpp
    imageview.h
    jpegavifconverter.cpp
    jpegavifconverter.h
    jpegheaderreader.cpp
    jpegheaderreader.h
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    myimagereader.cpp
    myimagereader.h
    resouce.qrc
)

set(TS_FILES
    app_en_US.ts
    app_zh_CN.ts
)

# Compile translations
qt6_add_translation(QM_FILES ${TS_FILES})

# Generate a qrc file to include the generated QM files under /translations prefix
set(TRANSLATIONS_QRC_PATH "${CMAKE_CURRENT_BINARY_DIR}/translations.qrc")

# We use a custom command to generate the qrc at build time or configure time.
# Configure time is easier for CMake integration.
set(QRC_CONTENT "<RCC>\n    <qresource prefix=\"/translations\">")
foreach(qm ${QM_FILES})
    get_filename_component(qm_name ${qm} NAME)
    string(APPEND QRC_CONTENT "        <file alias=\"${qm_name}\">${qm}</file>\n")
endforeach()
string(APPEND QRC_CONTENT "    </qresource>\n</RCC>")

file(WRITE ${TRANSLATIONS_QRC_PATH} "${QRC_CONTENT}")

# Add the generated QRC to the target
qt6_add_resources(PROJECT_RESOURCES ${TRANSLATIONS_QRC_PATH})

qt_add_executable(qavif-viewer
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES}
)

target_link_libraries(qavif-viewer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Concurrent
    avif
    $<IF:$<TARGET_EXISTS:libjpeg-turbo::turbojpeg>,libjpeg-turbo::turbojpeg,libjpeg-turbo::turbojpeg-static>
)

target_include_directories(qavif-viewer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(WIN32)
    target_link_libraries(qavif-viewer PRIVATE ws2_32 advapi32 psapi userenv)
endif()

# Installation
install(TARGETS qavif-viewer DESTINATION bin)