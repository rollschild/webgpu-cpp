#include <GLFW/glfw3.h>
#include <iostream>
#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h> // Non-standard from wgpu-native

int main() {
  // 1. create a descriptor
  // Descriptor is a kind of way to pack many function args together
  // Idiom:
  //   - first field of a descriptor is _always_ a pointer called `nextInChain`
  WGPUInstanceDescriptor desc = {};
  desc.nextInChain = nullptr;

  // 2. create an instance
  WGPUInstance instance = wgpuCreateInstance(&desc);

  // 3. check whether instance is created or not
  if (!instance) {
    std::cerr << "Could NOT initialize WebGPU!" << std::endl;
    return 1;
  }

  // 4. Display the object
  std::cout << "WGPU instance: " << instance << std::endl;

  // Any call to GLFW must happen between its initialization and termination
  if (!glfwInit()) {
    std::cerr << "Could NOT initialize GLFW!" << std::endl;
    return 1;
  }

  std::cout << "init!" << std::endl;

  GLFWwindow *window = glfwCreateWindow(640, 480, "WebGPU for C++", NULL, NULL);
  if (!window) {
    std::cerr << "Could NOT open window!" << std::endl;
    glfwTerminate();
    return 1;
  }
  std::cout << "window:" << window << std::endl;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  // 5. clean up the WebGPU instance
  wgpuInstanceDrop(instance);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
