cmake_minimum_required (VERSION 3.4)

project(SimpleAudioPlayer)

option( BUILD_CONSOLE_PLAYER "build console player project" OFF)

option( SUPPORT_OGG "" ON)
option( SUPPORT_MP3 "" OFF)
option( SUPPORT_WAV "" OFF)

add_subdirectory( src )

if (BUILD_CONSOLE_PLAYER)
    add_subdirectory(samples)
endif()

