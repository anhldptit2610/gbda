add_executable(gbda main.c)

target_link_libraries(gbda PRIVATE gbdacore
                                   SDL2)