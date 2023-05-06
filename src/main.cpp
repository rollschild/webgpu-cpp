#include "glfw3webgpu/glfw3webgpu.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>
#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h> // Non-standard from wgpu-native

/*
 * Util function to get a WebGPU adapter
 */
WGPUAdapter request_adapter(WGPUInstance instance,
                            WGPURequestAdapterOptions const *options) {
  struct UserData {
    WGPUAdapter adapter{nullptr};
    bool requestEnded{false};
  };
  UserData userData;

  auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status,
                                  WGPUAdapter adapter, char const *message,
                                  void *ptrUserData) {
    UserData &userData = *reinterpret_cast<UserData *>(ptrUserData);
    if (status == WGPURequestAdapterStatus_Success) {
      std::cout << "Successfully found adapter: " << adapter << std::endl;
      userData.adapter = adapter;
    } else {
      std::cout << "Could NOT get WebGPU adapter: " << message << std::endl;
    }
    userData.requestEnded = true;
  };

  wgpuInstanceRequestAdapter(instance, options, onAdapterRequestEnded,
                             (void *)&userData);

  assert(userData.requestEnded);

  return userData.adapter;
}

void inspect_adapter(WGPUAdapter adapter) {
  std::vector<WGPUFeatureName> features;
  size_t featureCount = wgpuAdapterEnumerateFeatures(adapter, nullptr);

  features.resize(featureCount);

  wgpuAdapterEnumerateFeatures(adapter, features.data());

  std::cout << "Adapter features: " << std::endl;
  for (auto f : features) {
    std::cout << " - " << f << std::endl;
  }
}

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

  // Tell GLFW _NOT_ to care about the graphics API setup,
  // as it does _NOT_ know WebGPU;
  // and we will _NOT_ use what it could set up by default for other APIs
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  GLFWwindow *window = glfwCreateWindow(640, 480, "WebGPU for C++", NULL, NULL);
  if (!window) {
    std::cerr << "Could NOT open window!" << std::endl;
    glfwTerminate();
    return 1;
  }
  std::cout << "window:" << window << std::endl;

  WGPUSurface surface = glfwGetWGPUSurface(instance, window);
  std::cout << "surface: " << surface << std::endl;
  std::cout << "Requesting adapter..." << std::endl;
  WGPURequestAdapterOptions adapterOptions{};
  adapterOptions.nextInChain = nullptr;
  // pass "the surface onto which we draw" as an option
  adapterOptions.compatibleSurface = surface;
  WGPUAdapter adapter = request_adapter(instance, &adapterOptions);

  std::cout << "Got adapter: " << adapter << std::endl;

  inspect_adapter(adapter);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  // 5. clean up the WebGPU instance
  wgpuInstanceDrop(instance);

  // Destroy the adapter
  // This function is wgpu-native specific
  wgpuAdapterDrop(adapter);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
