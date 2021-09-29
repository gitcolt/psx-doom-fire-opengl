add_executable(main main.c)

find_package(OpenGL REQUIRED)

find_package(GLEW REQUIRED)

find_package(glfw3 REQUIRED)

target_link_libraries(main
  PRIVATE
    m
    glfw
    OpenGL::GL
    GLEW::glew
  )
