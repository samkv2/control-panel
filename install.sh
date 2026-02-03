#!/bin/bash

echo "Installing Aero Control Panel..."

# Build
mkdir -p build
cd build
cmake ..
make -j$(nproc)

# Install files
echo "Installing binaries..."
sudo cp aero_panel_cpp /usr/bin/
sudo chmod +x /usr/bin/aero_panel_cpp

echo "Installing Desktop Entry..."
sudo cp ../aero_panel_cpp.desktop /usr/share/applications/

echo "Installing Icon..."
mkdir -p /usr/share/icons/hicolor/128x128/apps/
sudo cp ../resources/app_icon.png /usr/share/icons/hicolor/128x128/apps/aero_panel_cpp.png

echo "Done! You can search for 'Aero Control Panel' in your launcher."
