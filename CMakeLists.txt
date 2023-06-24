cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    CPP_SoftRender
    VERSION 0.0.1
    LANGUAGES CXX C
)

# cpu
add_compile_definitions(CPU_FEATURE_ENABLED)

# fetch SDL under Windows
# for Appveyor CI/CD
if (WIN32)
    include(cmake/fetch_sdl.cmake)
    option(FETCH_SDL "fetch SDL2 SDL2_image from web" FALSE)
    if (FETCH_SDL)
        FetchSDL()
    endif()
endif()

include(cmake/utility.cmake)

# find SDL
include(cmake/FindSDL2_image.cmake)
include(cmake/FindSDL2.cmake)

# some useful utiilities
include(cmake/copydll.cmake)
include(cmake/cppcheck.cmake)
AttachCppCheck()


# set(EXAMPLE_SRC example)
# aux_source_directory(src/example, EXAMPLE_SRC)

# build renderer

add_executable(${PROJECT_NAME} ./src/main.cpp)
target_link_libraries(${PROJECT_NAME}  PUBLIC SDL2 SDL2_image)
target_include_directories(${PROJECT_NAME}  PUBLIC include)

add_executable(draw_triangle ./src/example/1_draw_triangle.cpp)
target_link_libraries(draw_triangle  PUBLIC SDL2 SDL2_image)
target_include_directories(draw_triangle PUBLIC include)

add_executable(ortho ./src/example/2_ortho.cpp)
target_link_libraries(ortho  PUBLIC SDL2 SDL2_image)
target_include_directories(ortho PUBLIC include)

add_executable(perspective ./src/example/3_perspective.cpp)
target_link_libraries(perspective  PUBLIC SDL2 SDL2_image)
target_include_directories(perspective PUBLIC include)

add_executable(ortho_interactive ./src/example/4_ortho_interactive.cpp)
target_link_libraries(ortho_interactive  PUBLIC SDL2 SDL2_image)
target_include_directories(ortho_interactive PUBLIC include)

add_executable(deep_test ./src/example/5_deep_test.cpp)
target_link_libraries(deep_test  PUBLIC SDL2 SDL2_image)
target_include_directories(deep_test PUBLIC include)

add_executable(perspective_interactive ./src/example/6_perspective_interactive.cpp)
target_link_libraries(perspective_interactive  PUBLIC SDL2 SDL2_image)
target_include_directories(perspective_interactive PUBLIC include)

add_executable(texturemap_interactive ./src/example/7_texturemap_interactive.cpp)
target_link_libraries(texturemap_interactive  PUBLIC SDL2 SDL2_image)
target_include_directories(texturemap_interactive PUBLIC include)

add_executable(cube_interactive ./src/example/8_cube_interactive.cpp)
target_link_libraries(cube_interactive  PUBLIC SDL2 SDL2_image)
target_include_directories(cube_interactive PUBLIC include)

CopyDLL(${PROJECT_NAME})