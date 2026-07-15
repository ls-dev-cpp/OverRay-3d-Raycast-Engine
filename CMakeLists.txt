cmake_minimum_required(VERSION 3.14) # Recomendado 3.14+ para FetchContent estable
project(OverRay LANGUAGES CXX)

# Set the C++ standard (e.g., 11, 14, 17, 20, 23, or 26)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Traer dependencias externas primero (FetchContent)
include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY    https://github.com/g-truc/glm.git
    GIT_TAG           0af55ccecd98d4e5a8d1fad7de25ba429d60e863 # 1.0.1
)

# Esto descarga y añade el subproyecto de GLM al build
FetchContent_MakeAvailable(glm)

find_package(OpenMP REQUIRED)

# 2. Declarar tu ejecutable
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
add_executable(${PROJECT_NAME} WIN32 ${SOURCES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()

# 3. Configurar directorios de inclusión (SDL2 local)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/SDL2/include")

# 4. Enlazar librerías (SDL2 local y GLM descargado)
target_link_libraries(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/SDL2/lib/SDL2main.lib"
    "${CMAKE_SOURCE_DIR}/SDL2/lib/SDL2.lib"
    glm # GLM define el alias 'glm' directamente cuando se añade como subproyecto
)

# 5. Comandos post-compilación (Copiar DLL)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/SDL2/bin/SDL2.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)