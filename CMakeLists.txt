cmake_minimum_required(VERSION 3.16)

project(QtScreenSwitcher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Network)

# Windows specific libraries
if(WIN32)
    set(WIN_LIBS user32 gdi32 dxva2 shell32 advapi32)
endif()

set(PROJECT_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    ScreenManager.cpp
    ScreenManager.h
    TrayIcon.cpp
    TrayIcon.h
    ShortcutManager.cpp
    ShortcutManager.h
    SettingsManager.cpp
    SettingsManager.h
    resources.qrc
)

add_executable(QtScreenSwitcher WIN32 ${PROJECT_SOURCES})

target_link_libraries(QtScreenSwitcher PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core Qt6::Network ${WIN_LIBS})

# Set icon for windows
if(WIN32)
    # add_executable(QtScreenSwitcher WIN32 ${PROJECT_SOURCES} assets/icon.rc)
endif()
