cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(voice_brooch_nrf52)

target_sources(app PRIVATE
    src/main.c
    src/board_nrf52.c
    src/ble_gatt.c
    ../shared/vb_protocol.c
    ../shared/vb_button.c
    ../shared/vb_app.c
)

target_include_directories(app PRIVATE
    src
    ../shared
)
