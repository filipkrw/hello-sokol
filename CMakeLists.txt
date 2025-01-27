cmake_minimum_required(VERSION 3.10)
project(HelloSokol C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES )

if (APPLE) 
    list(APPEND SOURCES sokol_impl.m)
else()
    list(APPEND SOURCES sokol_impl.c)
endif()


add_executable(HelloSokol
    main.c
    ${SOURCES}
)

if(APPLE)
    target_link_libraries(HelloSokol
        "-framework Cocoa"
        # "-framework Metal"
        # "-framework MetalKit" 
        "-framework QuartzCore"
        "-framework IOKit"
        "-framework OpenGL"
    )
endif()