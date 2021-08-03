# HOW TO BUILD USING CMAKE as a standalone project:
# mkdir build
# cd build
# cmake .. -DCMAKE_TOOLCHAIN_FILE=path/to/Retro68-build/toolchain/m68k-apple-macos/cmake/retro68.toolchain.cmake
# make

project(MacJukebox)
cmake_minimum_required(VERSION 3.18)

add_application(MacJukebox
    main.c
    CONSOLE
   )

target_include_directories(MacJukebox PRIVATE ../Board/)

    # make the result as small as possible
    # by removing unused code (gc-sections)
    # and by removing macsbug function names on 68K
    # (don't do this when debugging...)
set_target_properties(MacJukebox PROPERTIES COMPILE_OPTIONS -ffunction-sections)
if(CMAKE_SYSTEM_NAME MATCHES Retro68)
    set_target_properties(MacJukebox PROPERTIES LINK_FLAGS "-Wl,-gc-sections -Wl,--mac-strip-macsbug")
else()
    set_target_properties(MacJukebox PROPERTIES LINK_FLAGS "-Wl,-gc-sections")
endif()
