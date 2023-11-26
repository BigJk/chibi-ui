cmake_minimum_required(VERSION 3.26)
project(chibi_ui)

set(CMAKE_CXX_STANDARD 17)

#
# Download CPM
#
file(
        DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
        EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

#
# Download GLFW
#
CPMAddPackage(
        NAME GLFW
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.3.8
        OPTIONS
        "GLFW_BUILD_TESTS OFF"
        "GLFW_BUILD_EXAMPLES OFF"
        "GLFW_BULID_DOCS OFF"
)

#
# Download ImGui
#
CPMAddPackage(
        NAME IMGUI
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG v1.90
)


FILE(GLOB IMGUI_SOURCES ${IMGUI_SOURCE_DIR}/*.cpp ${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp ${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)
add_definitions(-DIMGUI_DEFINE_MATH_OPERATORS)
add_compile_definitions(ImDrawIdx=int)

#
# Download quickjs
#
CPMAddPackage(
        NAME QUICKJS
        GITHUB_REPOSITORY quickjs-ng/quickjs
        GIT_TAG v0.1.0
)

#
# Find Packages
#
find_package(GLEW)
find_package(OpenGL)

if(NOT GLEW_FOUND)
    message("GLEW not found!!! Falling back to ENV variables...")

    set(GLEW_INCLUDE_DIRS $ENV{GLEW_INCLUDE})
    set(GLEW_LIBRARIES $ENV{GLEW_LIB})
endif()

#
# Exec
#
include_directories(./)
add_executable(chibi_ui ${IMGUI_SOURCES} main.cpp js_imgui.cpp js_util.cpp)

#
# Link
#
if(WIN32)
    if(GLEW_FOUND)
        target_link_libraries(chibi_ui ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} qjs GLEW::glew glfw)
    else()
        target_link_libraries(chibi_ui ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} qjs opengl32 glfw gdi32)
    endif()
else()
    target_link_libraries(chibi_ui ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} qjs GLEW::glew glfw)
endif()

target_include_directories(chibi_ui PRIVATE ${GLEW_INCLUDE_DIRS} ${IMGUI_INCLUDE_DIRS} ${QUICKJS_SOURCE_DIR} ${IMGUI_SOURCE_DIR})

if(GLFW_ADDED)
    target_include_directories(chibi_ui PRIVATE "${GLFW_SOURCE_DIR}/include")
endif()