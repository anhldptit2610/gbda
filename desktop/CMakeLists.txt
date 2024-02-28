add_executable(gbda main.c
                    sdl.c)

target_link_libraries(gbda PRIVATE gbdacore
                                   SDL2)