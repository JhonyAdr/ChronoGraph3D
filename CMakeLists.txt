cmake_minimum_required(VERSION 3.20)
project(ChronoGraph3D)

set(CMAKE_CXX_STANDARD 17)

if(NOT WIN32)
    message(FATAL_ERROR "ChronoGraph3D requires Windows (WinAPI)")
endif()

set(SOURCES
    main.cpp
    WndProc.cpp
    Renderer.cpp
    Parser.cpp
)

set(HEADERS
    ChronoGraph.h
)

# WIN32 = subsistema WinMain (sin consola)
add_executable(ChronoGraph3D WIN32 ${SOURCES} ${HEADERS})

target_compile_definitions(ChronoGraph3D PRIVATE UNICODE _UNICODE)

target_link_libraries(ChronoGraph3D
    user32
    gdi32
    comctl32
    comdlg32
    kernel32
)

if(MSVC)
    target_compile_options(ChronoGraph3D PRIVATE /W3)
else()
    target_compile_options(ChronoGraph3D PRIVATE
        -Wall -Wextra
        -Wno-missing-field-initializers
        -Wno-unused-parameter
        -Wno-unknown-pragmas
    )
endif()
