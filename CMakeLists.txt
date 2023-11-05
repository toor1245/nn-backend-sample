cmake_minimum_required(VERSION 3.26)
project(matrix_dotnet_core C)

set(CMAKE_C_STANDARD 99)

add_executable(matrix_dotnet_core main.c)
target_link_libraries(matrix_dotnet_core glfw vulkan)
