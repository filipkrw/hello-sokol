cmake_minimum_required(VERSION 3.10)
project(MySokolApp C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(MySokolApp
    main.c
    sokol_impl.m 
)

if(APPLE)
    target_link_libraries(MySokolApp
        "-framework Cocoa"
        # "-framework Metal"
        # "-framework MetalKit" 
        "-framework QuartzCore"
        "-framework IOKit"
        "-framework OpenGL"
    )
endif()