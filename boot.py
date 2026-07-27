import time
from machine import Pin, SPI
from camera import Camera, FrameSize, PixelFormat
import st7789
import vga1_8x16
import vga2_8x16
import prediktive_driver
import prediktive_library

button_last_press = 0
led_state = False

def button_handler(_):
    global button_last_press
    global led_state

    now = time.ticks_ms()

    if time.ticks_diff(now, button_last_press) < 200:
        return

    button_last_press = now

    led_state = not led_state

    prediktive_driver.set_led(led_state)

prediktive_driver.on_button_press(button_handler)

prediktive_driver.set_led(False)

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

display.fill(st7789.BLACK)

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

    fps = 0
    fps_frames = 0
    fps_timer = 0
    fps_last_frame_time_ms = time.ticks_ms()

    while True:
        fps_now_ms = time.ticks_ms()
        fps_dt_ms = fps_now_ms - fps_last_frame_time_ms
        fps_last_frame_time_ms = fps_now_ms

        fps_frames = prediktive_library.add(fps_frames, 1)
        fps_timer += fps_dt_ms / 1000
        
        if fps_timer >= 1:
            fps = fps_frames / fps_timer

            fps_frames = 0
            fps_timer = 0

        try:
            t0 = time.ticks_us()

            img = cam.capture()

            if img:
                t1 = time.ticks_us()

                display.blit_buffer(img, 0, 0, 320, 240)

                t2 = time.ticks_us()

                print('capture: {} ms | blit: {} ms | frame time: {} ms | fps: {}'.format(
                    time.ticks_diff(t1, t0) / 1000,
                    time.ticks_diff(t2, t1) / 1000,
                    fps_dt_ms,
                    fps
                ))
            else:
                print('Warning: Empty image buffer')
        except KeyboardInterrupt:
            print('\nStreaming interrupted by user')

            break
        except Exception as e:
            print('Error in video loop:', e)

            time.sleep(0.5)

        display.text(vga1_8x16, 'FPS: ', 10, 10, st7789.WHITE, st7789.BLACK)
        display.text(vga2_8x16, '{}'.format(fps), 50, 10, st7789.WHITE, st7789.BLACK)
