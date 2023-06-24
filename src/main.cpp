#include "glfw3webgpu.h"
#include <GLFW/glfw3.h>
#include <cstdint>
// #include <webgpu/webgpu.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu.hpp"
#include <webgpu/wgpu.h> // Non-standard from wgpu-native

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

using namespace wgpu;

int main() {
    // 1. create a descriptor
    // Descriptor is a kind of way to pack many function args together
    // Idiom:
    //   - first field of a descriptor is _always_ a pointer called
    //   `nextInChain`
    /* WGPUInstanceDescriptor desc = {};
    desc.nextInChain = nullptr; */

    // 2. create an instance
    Instance instance = createInstance(InstanceDescriptor{});

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

    // Tell the window to disable resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window =
        glfwCreateWindow(640, 480, "WebGPU for C++", NULL, NULL);
    if (!window) {
        std::cerr << "Could NOT open window!" << std::endl;
        glfwTerminate();
        return 1;
    }
    std::cout << "window:" << window << std::endl;

    Surface surface = glfwGetWGPUSurface(instance, window);
    std::cout << "surface: " << surface << std::endl;
    std::cout << "Requesting adapter..." << std::endl;
    RequestAdapterOptions adapterOptions{};
    // adapterOptions.nextInChain = nullptr;
    // pass "the surface onto which we draw" as an option
    adapterOptions.compatibleSurface = surface;
    Adapter adapter = instance.requestAdapter(adapterOptions);

    std::cout << "Got adapter: " << adapter << std::endl;

    std::cout << "Requesting device..." << std::endl;

    DeviceDescriptor deviceDesc{};
    // deviceDesc.nextInChain = nullptr;
    // label is used in error message for debugging
    // currently only used by Dawn
    deviceDesc.label = "My Device";
    deviceDesc.requiredFeaturesCount = 0;
    deviceDesc.requiredLimits = nullptr;
    // deviceDesc.defaultQueue.nextInChain = nullptr;
    deviceDesc.defaultQueue.label = "The default queue";

    Device device = adapter.requestDevice(deviceDesc);

    std::cout << "Got device: " << device << std::endl;

    // a callback that gets executed upon errors
    // convenient for _aysnc_ operations
    /* auto onDeviceError = [](WGPUErrorType type, char const *message,
                            void *ptrUserData) {
        std::cout << "Uncaptured device error: type " << type;
        if (message)
            std::cout << " (" << message << ")";
        std::cout << std::endl;
    };
    wgpuDeviceSetUncapturedErrorCallback(device, onDeviceError, nullptr); */
    auto h = device.setUncapturedErrorCallback(
        [](ErrorType type, char const *message) {
            std::cout << "Device error: type " << type;
            if (message)
                std::cout << " (message: " << message << ")";
            std::cout << std::endl;
        });

    // #ifdef WEBGPU_BACKEND_DAWN
    // auto onQueueWorkDone = [](WGPUQueueWorkDoneStatus status,
    // void *ptrUserData) {
    // std::cout << "Queue work finished with status: " << status << std::endl;
    // };
    // wgpuQueueOnSubmittedWorkDone(queue, 0 /* Non-standard arg for Dawn */,
    // onQueueWorkDone, nullptr /* ptrUserData */);
// #endif // WEBGPU_BACKEND_DAWN
#ifdef WEBGPU_BACKEND_WGPU
    TextureFormat swapChainFormat = surface.getPreferredFormat(adapter);
#else
    TextureFormat swapChainFormat = TextureFormat::BGRA8Unorm;
#endif // WEBGPU_BACKEND_WGPU

    /* // a WGPUCommandBuffer cannot be manually created
    // a *command encoder* is needed
    WGPUCommandEncoderDescriptor encoderDesc{};
    encoderDesc.nextInChain = nullptr;
    encoderDesc.label = "My command encoder";
    WGPUCommandEncoder encoder =
        wgpuDeviceCreateCommandEncoder(device, &encoderDesc);
    // use the encoder to write instructions
    wgpuCommandEncoderInsertDebugMarker(encoder, "Do one thing");
    wgpuCommandEncoderInsertDebugMarker(encoder, "Do another thing");

    // generates the command from the encoder
    WGPUCommandBufferDescriptor cmdBufferDesc{};
    cmdBufferDesc.nextInChain = nullptr;
    cmdBufferDesc.label = "Command buffer";
    // this operation destroys `encoder`
    WGPUCommandBuffer command =
        wgpuCommandEncoderFinish(encoder, &cmdBufferDesc);

    // submit the command queue
    std::cout << "Submitting command..." << std::endl;
    // destroys `command` buffer
    wgpuQueueSubmit(queue, 1, &command);

#ifdef WEBGPU_BACKEND_DAWN
    wgpuCommandEncoderRelease(encoder);
    wgpuCommandBufferRelease(command);
#endif // WEBGPU_BACKEND_DAWN */

    std::cout << "Allocating GPU memory..." << std::endl;

    // Create a GPU buffer
    BufferDescriptor bufferDesc{};
    bufferDesc.label = "Input buffer - written CPU -> GPU";
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::CopySrc;
    bufferDesc.size = 16;
    bufferDesc.mappedAtCreation = false;
    Buffer buffer1 = device.createBuffer(bufferDesc);

    bufferDesc.label = "Output buffer - read back GPU -> CPU";
    // add MapRead flag so that buffer can be mapped for reading
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::MapRead;
    bufferDesc.size = 16;
    bufferDesc.mappedAtCreation = false;
    Buffer buffer2 = device.createBuffer(bufferDesc);

    std::cout << "Configuring command queue..." << std::endl;

    // Get the queue - Our WebGPU device has a single queue
    // BUT in the future there might be multiple queues per device
    Queue queue = device.getQueue();

#if 0 // not yet implemented by backends
    auto onQueWorkDone = [](WGPUQueueWorkDoneStatus status, void*) {
        std::cout << "Queued work finished with status: " << status << std::endl;
    };
    wgpuQueueOnSubmittedWorkDone(queue, onQueueWorkDone, nullptr);
#endif

    std::cout << "Uploading data to GPU..." << std::endl;

    std::vector<uint8_t> numbers(16);
    for (uint8_t i{}; i < 16; ++i) {
        numbers[i] = i;
    }
    // copy from `numbers` (RAM) to `buffer1` (VRAM)
    queue.writeBuffer(buffer1, 0, numbers.data(), numbers.size());

    std::cout << "Sending buffer copy operation..." << std::endl;

    CommandEncoderDescriptor commandEncoderDesc{};
    commandEncoderDesc.label = "CommandEncoder";
    CommandEncoder encoder = device.createCommandEncoder(commandEncoderDesc);
    // add a command to the encoder
    // "Copy the current state of buffer1 to buffer2"
    encoder.copyBufferToBuffer(buffer1, 0, buffer2, 0, 16);

    // finalize the encoding operation, synthesizing the command buffer
    CommandBuffer command = encoder.finish(CommandBufferDescriptor{});
    queue.submit(1, &command);

    std::cout << "Start downloading result data from GPU..." << std::endl;

    // the context shared between this main function and the callback
    struct Context {
        Buffer buffer;
    };

    auto onBuffer2Mapped = [](WGPUBufferMapAsyncStatus status,
                              void *pUserData) {
        Context *context = reinterpret_cast<Context *>(pUserData);
        std::cout << "Buffer 2 mapped with status " << status << std::endl;
        if (status != BufferMapAsyncStatus::Success)
            return;

        // get a pointer to where the driver mapped the GPU memory to the RAM
        uint8_t *bufferData =
            (uint8_t *)context->buffer.getConstMappedRange(0, 16);

        // do stuff with bufferData
        std::cout << "bufferData = [";
        for (int i{}; i < 16; ++i) {
            if (i > 0)
                std::cout << ", ";
            std::cout << (int)bufferData[i];
        }
        std::cout << "]" << std::endl;

        // unmap the memory
        context->buffer.unmap();
    };
    Context context{buffer2};
    // maps the GPU buffer into CPU memory
    // whenever it is ready it executes the callback
    wgpuBufferMapAsync(buffer2, MapMode::Read, 0, 16, onBuffer2Mapped,
                       (void *)&context);

    while (!glfwWindowShouldClose(window)) {
        // regularly checks for ongoing async operations
        // and calls the callback

#ifdef WEBGPU_BACKEND_WGPU
        // NON-standard
        // submit empty queue to flush callbacks
        queue.submit(0, nullptr);
#else
        device.tick();
#endif
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    // Free up GPU memory
    wgpuSurfaceDrop(surface);
    // Make sure the calls to `buffer.destroy()` are issued _AFTER_ the main
    // loop otherwise callback will be called with status
    // `BufferMapAsyncStatus::DestroyedBeforeCallback`
    buffer1.destroy();
    buffer2.destroy();
    wgpuBufferDrop(buffer1);
    wgpuBufferDrop(buffer2);
    // buffer1.drop(); in the future?

    // 5. clean up the WebGPU instance
    wgpuInstanceDrop(instance);

    // Destroy the adapter
    // This function is wgpu-native specific
    wgpuAdapterDrop(adapter);

    wgpuDeviceDrop(device);

    return 0;
}
