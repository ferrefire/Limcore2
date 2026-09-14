cmake_minimum_required(VERSION 3.22)
project(Limcore VERSION 2.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FILE(GLOB SOURCE_FILES sources/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
#add_library(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC "includes")
target_compile_options(${PROJECT_NAME} PRIVATE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-O3>)

include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(BUILD_DEPS OFF CACHE BOOL "" FORCE)
set(UPDATE_DEPS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ENABLE_OPT OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG d9d6f0f1f967807ffade6598ea9a631ebaf37a56
	GIT_SHALLOW 1
)
FetchContent_MakeAvailable("glfw")
target_link_libraries(${PROJECT_NAME} PUBLIC "glfw")

FetchContent_Declare(
    Vulkan-Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG ee2ec5fd83dafce291024683b50dc89219333076
	GIT_SHALLOW 1
)
FetchContent_MakeAvailable("Vulkan-Headers")
target_link_libraries(${PROJECT_NAME} PUBLIC "Vulkan-Headers")

FetchContent_Declare(
    Vulkan-Loader
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader.git
    GIT_TAG b8b96a2862bff1eed468e602d43f706beae89cf1
	GIT_SHALLOW 1
)
FetchContent_MakeAvailable("Vulkan-Loader")
target_link_libraries(${PROJECT_NAME} PUBLIC "vulkan")

#FetchContent_Declare(
#    imgui
#    GIT_REPOSITORY https://github.com/ocornut/imgui.git
#    GIT_TAG b8b96a2862bff1eed468e602d43f706beae89cf1
#	GIT_SHALLOW 1
#)
#FetchContent_MakeAvailable("imgui")
#add_library(imgui STATIC
#	${imgui_SOURCE_DIR}/imgui.cpp
#	${imgui_SOURCE_DIR}/imgui_draw.cpp
#	${imgui_SOURCE_DIR}/imgui_tables.cpp
#	${imgui_SOURCE_DIR}/imgui_widgets.cpp
#
#	${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
#	${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
#)
#target_include_directories(imgui PUBLIC
#	${imgui_SOURCE_DIR}
#	${imgui_SOURCE_DIR}/backends
#)
#target_compile_options(imgui PRIVATE $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-O3>)
#target_link_libraries(imgui PUBLIC glfw)
#target_link_libraries(imgui PUBLIC Vulkan-Headers)
#target_link_libraries(imgui PUBLIC vulkan)
#target_link_libraries(${PROJECT_NAME} PUBLIC imgui)