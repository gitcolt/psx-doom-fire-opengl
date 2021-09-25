add_executable(main main.c)

find_package(OpenGL REQUIRED)

find_package(GLEW REQUIRED)

target_link_libraries(main
  PRIVATE
    glfw
    GL
    GLEW::glew
  )
