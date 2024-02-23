add_executable(gbda main.cpp)

target_link_libraries(gbda PRIVATE gbdacore
                                   SDL2
                                   imgui)