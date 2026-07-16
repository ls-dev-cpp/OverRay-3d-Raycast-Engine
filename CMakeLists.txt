cmake_minimum_required(VERSION 3.14)
project(OverRay LANGUAGES CXX)

# Set the C++ standard (e.g., 11, 14, 17, 20, 23, or 26)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY    https://github.com/g-truc/glm.git
    GIT_TAG           0af55ccecd98d4e5a8d1fad7de25ba429d60e863 # 1.0.1
)

FetchContent_MakeAvailable(glm)

find_package(OpenMP REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/SDL2/include")

target_link_libraries(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/SDL2/lib/SDL2main.lib"
    "${CMAKE_SOURCE_DIR}/SDL2/lib/SDL2.lib"
    glm
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/SDL2/bin/SDL2.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
