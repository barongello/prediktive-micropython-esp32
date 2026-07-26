add_library(usermod_prediktive_driver INTERFACE)

target_sources(usermod_prediktive_driver INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/prediktive_driver.c
)

target_include_directories(usermod_prediktive_driver INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(usermod_prediktive_driver INTERFACE idf::driver)

target_link_libraries(usermod INTERFACE usermod_prediktive_driver)
