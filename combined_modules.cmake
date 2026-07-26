include(${CMAKE_CURRENT_LIST_DIR}/deps/micropython-camera/micropython.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/deps/micropython-st7789/st7789/micropython.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/prediktive-sample-driver/micropython.cmake)

set(MICROPY_FROZEN_MANIFEST ${CMAKE_CURRENT_LIST_DIR}/manifest.py CACHE STRING "" FORCE)
