cmake_minimum_required(VERSION 3.16)

project(MacroPad VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(MacroPad
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        macro.h macro.cpp
        profile.h profile.cpp
        config.h config.cpp
    )
else()
    if(ANDROID)
        add_library(MacroPad SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(MacroPad
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(MacroPad PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Platform-Specific Configurations
if(APPLE)
    target_link_libraries(MacroPad PRIVATE "-framework Carbon" "-framework ApplicationServices")
    set_target_properties(MacroPad PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER com.example.MacroPad
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    )
elseif(WIN32)
    target_link_libraries(MacroPad PRIVATE user32.lib)
    set_target_properties(MacroPad PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(UNIX AND NOT APPLE) # Linux
    find_package(X11 REQUIRED)
    target_link_libraries(MacroPad PRIVATE X11)
endif()

include(GNUInstallDirs)
install(TARGETS MacroPad
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(MacroPad)
endif()
