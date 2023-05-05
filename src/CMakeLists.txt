# GLFW
find_package(glfw3 REQUIRED)

add_executable(App)
target_sources(App PRIVATE main.cpp)
target_link_libraries(App PRIVATE glfw webgpu)
set_target_properties(App PROPERTIES COMPILE_WARNING_AS_ERROR ON)
