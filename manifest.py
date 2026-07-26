include('$(PORT_DIR)/boards/manifest.py')
include('deps/micropython-camera/manifest.py')

freeze('prediktive-sample-driver', 'prediktive_driver.py')
freeze('prediktive-sample-library', 'prediktive_library.py')

freeze(
    'deps/micropython-st7789/fonts/bitmap',
    ('vga1_8x16.py', 'vga2_8x16.py')
)
