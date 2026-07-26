add_library(usermod_prediktive_library INTERFACE)

target_sources(usermod_prediktive_library INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/prediktive_library.c
)

target_include_directories(usermod_prediktive_library INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(usermod INTERFACE usermod_prediktive_library)
