# Prediktive's MicroPython for ESP32-WROVER-KIT with OV2640 camera and ST7789 display

## Environment

- ESP32-WROVER-KIT with OV2640 camera and ST7789 display
- Omarchy + Python 3.14.6
- ESP-IDF v5.5.2
  - Git: https://github.com/espressif/esp-idf
  - Commit: `30aaf64524299d3bde422ca9a2848090d1bc5d0f`
- MicroPython 1.28.0
  - Git: https://github.com/micropython/micropython
  - Commit: `b9cceaa6fd61d9620e96296730156c8a82c6802f`
- MicroPython Camera API v0.6.2
  - Git: https://github.com/cnadler86/micropython-camera-API
  - Commit: `8df3f70b20e52adad24f3f3de42628366f11cc06`
- MicroPython ST7789
  - Git: https://github.com/russhughes/st7789_mpy
  - Commit: `0ea2f739319a84225411f69aa85d339c261b8185`

## Clone this repository and submodules

```bash
git clone --recurse-submodules --shallow-submodules https://github.com/barongello/prediktive-micropython-esp32.git
```

Or:

```bash
git clone https://github.com/barongello/prediktive-micropython-esp32.git
cd prediktive-micropython-esp32
git submodule update --init --recursive --depth 1
```

## Increase the SPI speed

Out of the box, the SPI is limited to `26.6MHz` and the `blit_buffer` takes `~160ms` per call. After the below changes, it executes in `80MHz` and takes `~20ms`, 87.5% faster

### Modify the deps/micropython/ports/esp32/machine_hw_spi.c

In the `buscfg` struct, it is missing the `.max_transfer_sz` property, let's assign it to `320 * 240 * 2 + 8` to acommodate our frame (plus some extra). So, let's change from:

```cpp
spi_bus_config_t buscfg = {
  .miso_io_num = self->miso,
  .mosi_io_num = self->mosi,
  .sclk_io_num = self->sck,
  .quadwp_io_num = -1,
  .quadhd_io_num = -1
};
```

To:

```cpp
spi_bus_config_t buscfg = {
  .miso_io_num = self->miso,
  .mosi_io_num = self->mosi,
  .sclk_io_num = self->sck,
  .quadwp_io_num = -1,
  .quadhd_io_num = -1,
  .max_transfer_sz = 320 * 240 * 2 + 8
};
```

In the `devcfg` struct, the `.flags` property needs to always receive `SPI_DEVICE_NO_DUMMY`. So, let's change from:

```cpp
spi_device_interface_config_t devcfg = {
  .clock_speed_hz = self->baudrate,
  .mode = self->phase | (self->polarity << 1),
  .spics_io_num = -1, // No CS pin
  .queue_size = 2,
  .flags = self->firstbit == MICROPY_PY_MACHINE_SPI_LSB ? SPI_DEVICE_TXBIT_LSBFIRST | SPI_DEVICE_RXBIT_LSBFIRST : 0,
  .pre_cb = NULL
}
```

To:

```cpp
spi_device_interface_config_t devcfg = {
  .clock_speed_hz = self->baudrate,
  .mode = self->phase | (self->polarity << 1),
  .spics_io_num = -1, // No CS pin
  .queue_size = 2,
  .flags = (self->firstbit == MICROPY_PY_MACHINE_SPI_LSB ? SPI_DEVICE_TXBIT_LSBFIRST | SPI_DEVICE_RXBIT_LSBFIRST : 0) | SPI_DEVICE_NO_DUMMY,
  .pre_cb = NULL
}
```

### Modify the deps/micropython-st7789/st7789/st7789.c

The `blit_buffer` function has a small `buf_size`, which leads to a lot of calls to `write_spi` function, adding a lot of overhead. Let's find the function and patch the `buf_size` directly to acommodate our frame data. So, let's change from:

```cpp
static mp_obj_t st7789_ST7789_blit_buffer(size_t n_args, const mp_obj_t *args) {
  // ...

  const int buf_size = 256;

  // ...

  return mp_const_none;
}
```

To:

```cpp
static mp_obj_t st7789_ST7789_blit_buffer(size_t n_args, const mp_obj_t *args) {
  // ...

  const int buf_size = 320 * 240 * 2;

  // ...

  return mp_const_none;
}
```

## Compile the firmware

```bash
cd deps
  cd esp-idf
    ./install.sh
    source ./export.sh
    cd ..
  cd micropython
    cd ports
      cd esp32
        idf.py -B build-prediktive \
          -D MICROPY_BOARD=ESP32_GENERIC \
          -D MICROPY_BOARD_VARIANT=SPIRAM \
          -D MICROPY_CAMERA_MODEL=WROVER_KIT \
          -D USER_C_MODULES=$(realpath ../../../../combined_modules.cmake) \
          -D EXTRA_COMPONENT_DIRS=$(realpath ../../../micropython-camera) \
          build
        cd build-prediktive
          python ../makeimg.py sdkconfig \
            bootloader/bootloader.bin \
            partition_table/partition-table.bin \
            micropython.bin \
            firmware.bin \
            micropython.uf2
```

## Upload the new firmware to the board

```bash
cd deps
  cd micropython
    cd ports
      cd esp32
        cd build-prediktive
          esptool.py --port <device port> erase_flash
          esptool.py --port <device port> --chip esp32 -b 460800 write_flash -z 0x1000 firmware.bin
```

## Update board's boot.py

```python
import time
from machine import Pin, SPI
from camera import Camera, FrameSize, PixelFormat
import st7789

print('Initializing ST7789 display...')

spi = SPI(
    2,
    baudrate=80000000,
    polarity=1,
    phase=1,
    sck=Pin(14),
    mosi=Pin(13)
)

display = st7789.ST7789(
    spi, 
    240, 320,
    reset=Pin(2, Pin.OUT), 
    dc=Pin(3, Pin.OUT), 
    cs=Pin(15, Pin.OUT),
    rotation=1
)

display.init()

BLACK = st7789.color565(0, 0, 0)

display.fill(BLACK)

print('Initializing camera...')

try:
    cam = Camera(pixel_format=PixelFormat.RGB565, frame_size=FrameSize.QVGA)

    time.sleep(2.0)                      

    print('Camera initialized!')
except Exception as e:
    print("Error initializing camera:", e)

    cam = None

if cam:
    print('Starting real time video streaming...')

    while True:
        try:
            t0 = time.ticks_us()

            img = cam.capture()

            if img:
                t1 = time.ticks_us()

                display.blit_buffer(img, 0, 0, 320, 240)

                t2 = time.ticks_us()

                print('capture: {} ms | blit: {} ms'.format(
                    time.ticks_diff(t1, t0) / 1000,
                    time.ticks_diff(t2, t1) / 1000
                ))
            else:
                print('Warning: Empty image buffer')
        except KeyboardInterrupt:
            print('\nStreaming interrupted by user')

            break
        except Exception as e:
            print('Error in video loop:', e)

            time.sleep(0.5)
```

---

## How to create this repository

```bash
git init

git submodule add --depth 1 https://github.com/espressif/esp-idf.git deps/esp-idf
cd deps/esp-idf
git fetch --depth 1 origin 30aaf64524299d3bde422ca9a2848090d1bc5d0f
git checkout 30aaf64524299d3bde422ca9a2848090d1bc5d0f
cd ../..
git config -f .gitmodules submodule.deps/esp-idf.shallow true
git add deps/esp-idf .gitmodules
git commit -m "Pin esp-idf (shallow) to 30aaf64524299d3bde422ca9a2848090d1bc5d0f"

git submodule add --depth 1 https://github.com/micropython/micropython.git deps/micropython
cd deps/micropython
git fetch --depth 1 origin b9cceaa6fd61d9620e96296730156c8a82c6802f
git checkout b9cceaa6fd61d9620e96296730156c8a82c6802f
cd ../..
git config -f .gitmodules submodule.deps/micropython.shallow true
git add deps/micropython .gitmodules
git commit -m "Pin micropython (shallow) to b9cceaa6fd61d9620e96296730156c8a82c6802f"

git submodule add --depth 1 https://github.com/cnadler86/micropython-camera-API.git deps/micropython-camera
cd deps/micropython-camera
git fetch --depth 1 origin 8df3f70b20e52adad24f3f3de42628366f11cc06
git checkout 8df3f70b20e52adad24f3f3de42628366f11cc06
cd ../..
git config -f .gitmodules submodule.deps/micropython-camera.shallow true
git add deps/micropython-camera .gitmodules
git commit -m "Pin micropython-camera (shallow) to 8df3f70b20e52adad24f3f3de42628366f11cc06"

git submodule add --depth 1 https://github.com/russhughes/st7789_mpy.git deps/micropython-st7789
cd deps/micropython-st7789
git fetch --depth 1 origin 0ea2f739319a84225411f69aa85d339c261b8185
git checkout 0ea2f739319a84225411f69aa85d339c261b8185
cd ../..
git config -f .gitmodules submodule.deps/micropython-st7789.shallow true
git add deps/micropython-st7789 .gitmodules
git commit -m "Pin micropython-st7789 (shallow) to 0ea2f739319a84225411f69aa85d339c261b8185"

git branch -M main

git remote add origin git@github.com:barongello/prediktive-micropython-esp32.git

git push -u origin main
```
