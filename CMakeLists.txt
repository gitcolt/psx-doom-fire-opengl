cmake_minimum_required(VERSION 3.0)
project(doom-fire)

cmake_policy(SET CMP0072 NEW)

add_subdirectory(src)

file(GLOB MY_FILES "shader/*.glsl")
file(COPY ${MY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shader)
file(GLOB MY_FILES "assets/*")
file(COPY ${MY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/assets)
