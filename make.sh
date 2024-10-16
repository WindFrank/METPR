#!/bin/bash
export VGENERATOR_WORK_DIR="$(pwd)"
mkdir build
cd build
cmake ..
make
cd ..
if sudo cp "$(pwd)/build/Vgenerator" /usr/bin/Vgenerator; then
    sudo chmod +x /usr/bin/Vgenerator
    echo "Vgenerator has been successfully installed."
else
    echo "Failed to install Vgenerator."
fi
