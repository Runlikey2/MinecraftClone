cmake_minimum_required(VERSION 3.24)
project(MinecraftClone VERSION 0.1.0 LANGUAGES C CXX)

# ── C++20 ───────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Paths ───────────────────────────────────────────────────────
set(DEPS_DIR "${CMAKE_SOURCE_DIR}/deps")

# ── GLFW (built from source, skip examples/tests/docs) ─────────
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory("${DEPS_DIR}/glfw" "${CMAKE_BINARY_DIR}/glfw")

# ── GLAD (compiled as a small static lib) ───────────────────────
add_library(glad STATIC "${DEPS_DIR}/glad/src/glad.c")
target_include_directories(glad PUBLIC "${DEPS_DIR}/glad/include")

# ── Gather engine sources ──────────────────────────────────────
file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

# ── Executable ──────────────────────────────────────────────────
add_executable(${PROJECT_NAME} ${ENGINE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
    "${DEPS_DIR}/glad/include"
    "${DEPS_DIR}/glm"
    "${DEPS_DIR}"                  # for FastNoiseLite.h & stb_image.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad)

# ── Platform libs ───────────────────────────────────────────────
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
elseif(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL dl pthread)
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)
endif()

# ── Copy assets to build directory ──────────────────────────────
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets to output directory"
)

# ── Compiler warnings ──────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
