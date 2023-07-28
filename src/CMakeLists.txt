# GLFW
find_package(glfw3 REQUIRED)

option(DEV_MODE "Set up development helper settings" ON)

add_executable(App main.cpp)
target_link_libraries(App PRIVATE glfw webgpu glfw3webgpu)
set_target_properties(App PROPERTIES COMPILE_WARNING_AS_ERROR ON)
target_copy_webgpu_binaries(App)

if(DEV_MODE)
  # Get the root directory, which is one level above `resources/`
  cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH ROOT_DIR)
  target_compile_definitions(App PRIVATE RESOURCE_DIR="${ROOT_DIR}/resources")
else()
  target_compile_definitions(App PRIVATE RESOURCE_DIR="./resources")
endif()
