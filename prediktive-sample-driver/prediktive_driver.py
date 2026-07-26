import _prediktive_driver

_prediktive_driver.init()

def set_led(state):
    _prediktive_driver.led_set(1 if state else 0)

def read_button():
    return _prediktive_driver.button_read()

def on_button_press(callback):
    _prediktive_driver.set_button_callback(callback)
