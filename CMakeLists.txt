cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(
    CPP_SoftRender
    VERSION 0.0.1
    LANGUAGES CXX C
)

# cpu
add_compile_definitions(GPU_FEATURE_ENABLED)

# fetch SDL under Windows
# for Appveyor CI/CD
if(WIN32)
    include(cmake/fetch_sdl.cmake)
    option(FETCH_SDL "fetch SDL2 SDL2_image from web" FALSE)

    if(FETCH_SDL)
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

# build renderer
add_executable(${PROJECT_NAME} ./src/main.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2 PUBLIC SDL2_image)
target_include_directories(${PROJECT_NAME} PUBLIC include)

CopyDLL(${PROJECT_NAME})