
cmake_minimum_required(VERSION 3.16)

project(aero_panel_cpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

qt_standard_project_setup()

# Resources
qt_add_resources(Resources resources/resources.qrc)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/pages/HomePage.h
    src/pages/HomePage.cpp
    src/pages/ProgramsPage.h
    src/pages/ProgramsPage.cpp
    src/pages/SystemPage.h
    src/pages/SystemPage.cpp
    src/pages/UserPage.h
    src/pages/UserPage.cpp
    src/pages/SettingsPage.h
    src/pages/SettingsPage.cpp
    src/pages/StartupPage.h
    src/pages/StartupPage.cpp
)

qt_add_executable(aero_panel_cpp
    ${SOURCES}
    ${Resources}
)

target_link_libraries(aero_panel_cpp PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)

# Asset Copying (Icons for dev)
file(COPY resources/icons DESTINATION ${CMAKE_BINARY_DIR}/resources)

# Installation
install(TARGETS aero_panel_cpp DESTINATION bin)
install(FILES aero_panel_cpp.desktop DESTINATION share/applications)
install(FILES resources/app_icon.png DESTINATION share/icons/hicolor/128x128/apps RENAME aero_panel_cpp.png)

# CPack Packaging
set(CPACK_PACKAGE_NAME "aero-control-panel")
set(CPACK_PACKAGE_VENDOR "Andromeda")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A modern Qt-based Control Panel with Aero aesthetics")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "support@andromeda.os")

# DEB Config
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andromeda")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libqt6widgets6 (>= 6.0), libqt6core6 (>= 6.0), libqt6gui6 (>= 6.0)")

include(CPack)
