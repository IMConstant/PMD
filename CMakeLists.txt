cmake_minimum_required(VERSION 3.10)
project(MeshSimplification)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory(./dependencies/glew/build/cmake)
add_subdirectory(./dependencies/glfw)
add_subdirectory(./dependencies/imgui/imgui-master)
#add_subdirectory(./dependencies/CGAL-5.2.1)

add_executable(MeshSimplification src/main.cpp src/Application.cpp src/Application.h src/Mesh.cpp src/Mesh.h src/Shader.h src/Shader.cpp src/Simplify.h src/Simplify.cpp src/Input.h src/Input.cpp src/OBJReader.h)
target_include_directories(MeshSimplification PUBLIC ./dependencies/glew/include
        ./dependencies/glfw/include
        ./dependencies/glm
        ./dependencies/imgui/imgui-master
        ./dependencies/stb_image)
target_link_libraries(MeshSimplification PUBLIC glew_s glfw imgui CGAL)