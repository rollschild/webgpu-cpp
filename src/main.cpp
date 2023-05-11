#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <webgpu/wgpu.h> // Non-standard from wgpu-native

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

#include "glfw3webgpu/glfw3webgpu.h"

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
            std::cout << "Could NOT get WebGPU adapter: " << message
                      << std::endl;
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

    WGPUSupportedLimits limits{};
    limits.nextInChain = nullptr;
    bool success = wgpuAdapterGetLimits(adapter, &limits);
    if (success) {
        std::cout << "Adapter limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: "
                  << limits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: "
                  << limits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: "
                  << limits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: "
                  << limits.limits.maxTextureArrayLayers << std::endl;
        std::cout << " - maxBindGroups: " << limits.limits.maxBindGroups
                  << std::endl;
        std::cout << " - maxDynamicUniformBuffersPerPipelineLayout: "
                  << limits.limits.maxDynamicUniformBuffersPerPipelineLayout
                  << std::endl;
        std::cout << " - maxDynamicStorageBuffersPerPipelineLayout: "
                  << limits.limits.maxDynamicStorageBuffersPerPipelineLayout
                  << std::endl;
        std::cout << " - maxSampledTexturesPerShaderStage: "
                  << limits.limits.maxSampledTexturesPerShaderStage
                  << std::endl;
        std::cout << " - maxSamplersPerShaderStage: "
                  << limits.limits.maxSamplersPerShaderStage << std::endl;
        std::cout << " - maxStorageBuffersPerShaderStage: "
                  << limits.limits.maxStorageBuffersPerShaderStage << std::endl;
        std::cout << " - maxStorageTexturesPerShaderStage: "
                  << limits.limits.maxStorageTexturesPerShaderStage
                  << std::endl;
        std::cout << " - maxUniformBuffersPerShaderStage: "
                  << limits.limits.maxUniformBuffersPerShaderStage << std::endl;
        std::cout << " - maxUniformBufferBindingSize: "
                  << limits.limits.maxUniformBufferBindingSize << std::endl;
        std::cout << " - maxStorageBufferBindingSize: "
                  << limits.limits.maxStorageBufferBindingSize << std::endl;
        std::cout << " - minUniformBufferOffsetAlignment: "
                  << limits.limits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << " - minStorageBufferOffsetAlignment: "
                  << limits.limits.minStorageBufferOffsetAlignment << std::endl;
        std::cout << " - maxVertexBuffers: " << limits.limits.maxVertexBuffers
                  << std::endl;
        std::cout << " - maxVertexAttributes: "
                  << limits.limits.maxVertexAttributes << std::endl;
        std::cout << " - maxVertexBufferArrayStride: "
                  << limits.limits.maxVertexBufferArrayStride << std::endl;
        std::cout << " - maxInterStageShaderComponents: "
                  << limits.limits.maxInterStageShaderComponents << std::endl;
        std::cout << " - maxComputeWorkgroupStorageSize: "
                  << limits.limits.maxComputeWorkgroupStorageSize << std::endl;
        std::cout << " - maxComputeInvocationsPerWorkgroup: "
                  << limits.limits.maxComputeInvocationsPerWorkgroup
                  << std::endl;
        std::cout << " - maxComputeWorkgroupSizeX: "
                  << limits.limits.maxComputeWorkgroupSizeX << std::endl;
        std::cout << " - maxComputeWorkgroupSizeY: "
                  << limits.limits.maxComputeWorkgroupSizeY << std::endl;
        std::cout << " - maxComputeWorkgroupSizeZ: "
                  << limits.limits.maxComputeWorkgroupSizeZ << std::endl;
        std::cout << " - maxComputeWorkgroupsPerDimension: "
                  << limits.limits.maxComputeWorkgroupsPerDimension
                  << std::endl;
    }

    WGPUAdapterProperties properties{};
    properties.nextInChain = nullptr;
    wgpuAdapterGetProperties(adapter, &properties);
    std::cout << "Adapter properties:" << std::endl;
    std::cout << " - vendorID: " << properties.vendorID << std::endl;
    std::cout << " - deviceID: " << properties.deviceID << std::endl;
    std::cout << " - name: " << properties.name << std::endl;
    if (properties.driverDescription) {
        std::cout << " - driverDescription: " << properties.driverDescription
                  << std::endl;
    }
    std::cout << " - adapterType: " << properties.adapterType << std::endl;
    std::cout << " - backendType: " << properties.backendType << std::endl;
}

WGPUDevice requestDevice(WGPUAdapter adapter,
                         WGPUDeviceDescriptor const *descriptor) {
    struct UserData {
        WGPUDevice device{nullptr};
        bool requestEnded{false};
    };
    UserData userData;

    auto onDeviceRequestEnded = [](WGPURequestDeviceStatus status,
                                   WGPUDevice device, char const *message,
                                   void *ptrUserData) {
        UserData &userData = *reinterpret_cast<UserData *>(ptrUserData);
        if (status == WGPURequestDeviceStatus_Success) {
            userData.device = device;
        } else {
            std::cout << "Could NOT get WebGPU adapter!" << message
                      << std::endl;
        }
        userData.requestEnded = true;
    };

    wgpuAdapterRequestDevice(adapter, descriptor, onDeviceRequestEnded,
                             (void *)&userData);

    assert(userData.requestEnded);

    return userData.device;
}

void inspectDevice(WGPUDevice device) {
    std::vector<WGPUFeatureName> features{};
    size_t featureCount = wgpuDeviceEnumerateFeatures(device, nullptr);
    features.resize(featureCount);
    wgpuDeviceEnumerateFeatures(device, features.data());

    std::cout << "Device features: " << std::endl;
    for (auto f : features) {
        std::cout << " - " << f << std::endl;
    }

    WGPUSupportedLimits limits{};
    limits.nextInChain = nullptr;
    bool success = wgpuDeviceGetLimits(device, &limits);
    if (success) {
        std::cout << "Device limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: "
                  << limits.limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: "
                  << limits.limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: "
                  << limits.limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: "
                  << limits.limits.maxTextureArrayLayers << std::endl;
        std::cout << " - maxBindGroups: " << limits.limits.maxBindGroups
                  << std::endl;
        std::cout << " - maxDynamicUniformBuffersPerPipelineLayout: "
                  << limits.limits.maxDynamicUniformBuffersPerPipelineLayout
                  << std::endl;
        std::cout << " - maxDynamicStorageBuffersPerPipelineLayout: "
                  << limits.limits.maxDynamicStorageBuffersPerPipelineLayout
                  << std::endl;
        std::cout << " - maxSampledTexturesPerShaderStage: "
                  << limits.limits.maxSampledTexturesPerShaderStage
                  << std::endl;
        std::cout << " - maxSamplersPerShaderStage: "
                  << limits.limits.maxSamplersPerShaderStage << std::endl;
        std::cout << " - maxStorageBuffersPerShaderStage: "
                  << limits.limits.maxStorageBuffersPerShaderStage << std::endl;
        std::cout << " - maxStorageTexturesPerShaderStage: "
                  << limits.limits.maxStorageTexturesPerShaderStage
                  << std::endl;
        std::cout << " - maxUniformBuffersPerShaderStage: "
                  << limits.limits.maxUniformBuffersPerShaderStage << std::endl;
        std::cout << " - maxUniformBufferBindingSize: "
                  << limits.limits.maxUniformBufferBindingSize << std::endl;
        std::cout << " - maxStorageBufferBindingSize: "
                  << limits.limits.maxStorageBufferBindingSize << std::endl;
        std::cout << " - minUniformBufferOffsetAlignment: "
                  << limits.limits.minUniformBufferOffsetAlignment << std::endl;
        std::cout << " - minStorageBufferOffsetAlignment: "
                  << limits.limits.minStorageBufferOffsetAlignment << std::endl;
        std::cout << " - maxVertexBuffers: " << limits.limits.maxVertexBuffers
                  << std::endl;
        std::cout << " - maxVertexAttributes: "
                  << limits.limits.maxVertexAttributes << std::endl;
        std::cout << " - maxVertexBufferArrayStride: "
                  << limits.limits.maxVertexBufferArrayStride << std::endl;
        std::cout << " - maxInterStageShaderComponents: "
                  << limits.limits.maxInterStageShaderComponents << std::endl;
        std::cout << " - maxComputeWorkgroupStorageSize: "
                  << limits.limits.maxComputeWorkgroupStorageSize << std::endl;
        std::cout << " - maxComputeInvocationsPerWorkgroup: "
                  << limits.limits.maxComputeInvocationsPerWorkgroup
                  << std::endl;
        std::cout << " - maxComputeWorkgroupSizeX: "
                  << limits.limits.maxComputeWorkgroupSizeX << std::endl;
        std::cout << " - maxComputeWorkgroupSizeY: "
                  << limits.limits.maxComputeWorkgroupSizeY << std::endl;
        std::cout << " - maxComputeWorkgroupSizeZ: "
                  << limits.limits.maxComputeWorkgroupSizeZ << std::endl;
        std::cout << " - maxComputeWorkgroupsPerDimension: "
                  << limits.limits.maxComputeWorkgroupsPerDimension
                  << std::endl;
    }
}

int main() {
    // 1. create a descriptor
    // Descriptor is a kind of way to pack many function args together
    // Idiom:
    //   - first field of a descriptor is _always_ a pointer called
    //   `nextInChain`
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

    GLFWwindow *window =
        glfwCreateWindow(640, 480, "WebGPU for C++", NULL, NULL);
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

    std::cout << "Requesting device..." << std::endl;

    WGPUDeviceDescriptor deviceDesc{};
    deviceDesc.nextInChain = nullptr;
    // label is used in error message for debugging
    // currently only used by Dawn
    deviceDesc.label = "My Device";
    deviceDesc.requiredFeaturesCount = 0;
    deviceDesc.requiredLimits = nullptr;
    deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "The default queue";

    WGPUDevice device = requestDevice(adapter, &deviceDesc);

    std::cout << "Got device: " << device << std::endl;

    // a callback that gets executed upon errors
    // convenient for _aysnc_ operations
    auto onDeviceError = [](WGPUErrorType type, char const *message,
                            void *ptrUserData) {
        std::cout << "Uncaptured device error: type " << type;
        if (message)
            std::cout << " (" << message << ")";
        std::cout << std::endl;
    };
    wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr);

    inspectDevice(device);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    // 5. clean up the WebGPU instance
    wgpuInstanceDrop(instance);

    // Destroy the adapter
    // This function is wgpu-native specific
    wgpuAdapterDrop(adapter);

    wgpuDeviceDrop(device);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
