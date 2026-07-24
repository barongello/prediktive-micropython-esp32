#!/usr/bin/env bash

set -e

echo "Initializing submodules..."
git submodule update --init --recursive --depth 1

echo "Applying patches..."
cd deps/micropython
git apply ../../patches/micropython-machine_hw_spi-max-transfer-sz.patch
git apply ../../patches/micropython-machine_hw_spi-no-dummy-flag.patch
cd ../micropython-st7789
git apply ../../patches/micropython-st7789-blit-buffer-size.patch

echo "Configuring environment..."
cd ../esp-idf
./install.sh
source ./export.sh

echo "Compiling MicroPython..."
cd ../micropython/ports/esp32
idf.py -B build-prediktive \
  -D MICROPY_BOARD=ESP32_GENERIC \
  -D MICROPY_BOARD_VARIANT=SPIRAM \
  -D MICROPY_CAMERA_MODEL=WROVER_KIT \
  -D USER_C_MODULES=$(realpath ../../../../combined_modules.cmake) \
  -D EXTRA_COMPONENT_DIRS=$(realpath ../../../micropython-camera) \
  build

echo "Making the firmware..."
cd build-prediktive
python ../makeimg.py sdkconfig \
  bootloader/bootloader.bin \
  partition_table/partition-table.bin \
  micropython.bin \
  firmware.bin \
  micropython.uf2

echo "Done!"
