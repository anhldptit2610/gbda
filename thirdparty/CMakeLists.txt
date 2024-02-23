add_library(imgui SHARED
    ./imgui/imgui.cpp
    ./imgui/imgui.h
    ./imgui/imconfig.h
    ./imgui/imgui_demo.cpp
    ./imgui/imgui_draw.cpp
    ./imgui/imgui_internal.h
    ./imgui/imgui_tables.cpp
    ./imgui/imgui_widgets.cpp
    ./imgui/imstb_rectpack.h
    ./imgui/imstb_textedit.h
    ./imgui/imstb_truetype.h
    ./imgui/backends/imgui_impl_sdl2.h
    ./imgui/backends/imgui_impl_sdl2.cpp
    ./imgui/backends/imgui_impl_sdlrenderer2.h
    ./imgui/backends/imgui_impl_sdlrenderer2.cpp)

target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(imgui PUBLIC ./imgui/ ./imgui/backends)
