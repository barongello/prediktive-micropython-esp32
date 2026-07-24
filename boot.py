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
