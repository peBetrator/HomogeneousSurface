cmake_minimum_required(VERSION 3.15)
project(HomogeneousSurface)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

# Include directories
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    HomogeneousSurface/Project0.cpp
    HomogeneousSurface/Background.cpp
    HomogeneousSurface/CoordinateAxes.cpp
    HomogeneousSurface/AstroidCone.cpp
    HomogeneousSurface/AstroidConeLit.cpp
)

# Header files
set(HEADERS
    HomogeneousSurface/Background.hpp
    HomogeneousSurface/CoordinateAxes.hpp
    HomogeneousSurface/AstroidCone.hpp
    HomogeneousSurface/AstroidConeLit.hpp
    HomogeneousSurface/ISurface3D.hpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    OpenGL::GL
    OpenGL::GLU
    glfw
)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
