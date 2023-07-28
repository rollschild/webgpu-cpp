#include "glfw3webgpu.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
// #include <webgpu/webgpu.h>
#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu.hpp"
#include <webgpu/wgpu.h> // Non-standard from wgpu-native

#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

using namespace wgpu;

namespace fs = std::filesystem;

bool loadGeometry(const fs::path &path, std::vector<float> &pointData,
                  std::vector<uint16_t> &indexData) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    pointData.clear();
    indexData.clear();

    enum class Section {
        None,
        Points,
        Indices,
    };
    Section currentSection = Section::None;

    float value;
    uint16_t index;
    std::string line{};
    while (!file.eof()) {
        std::getline(file, line);

        // fix the `CRLF` problem
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line == "[points]") {
            currentSection = Section::Points;
        } else if (line == "[indices]") {
            currentSection = Section::Indices;
        } else if (line[0] == '#' || line.empty()) {
            // Do nothing
        } else if (currentSection == Section::Points) {
            std::istringstream iss(line);
            // Get x, y, r, g, b
            for (int i = 0; i < 5; ++i) {
                iss >> value;
                pointData.push_back(value);
            }
        } else if (currentSection == Section::Indices) {
            std::istringstream iss(line);
            // Get corners #0, #1, and #2
            for (int i = 0; i < 3; ++i) {
                iss >> index;
                indexData.push_back(index);
            }
        }
    }

    return true;
}

ShaderModule loadShaderModule(const fs::path &path, Device device) {
    // input operations on file-based systems
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }

    // the following two lines of code basically tires to get the length of the
    // code (text) in the file, in terms of number of characters
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    // construct a sring of size `size`, with all `' '` characters
    std::string shaderSource(size, ' ');
    file.seekg(0);
    // read `size` number of characters to char array pointed to by
    // `shaderSource.data()`
    file.read(shaderSource.data(), size);
    ShaderModuleWGSLDescriptor shaderCodeDesc{};
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    ShaderModuleDescriptor shaderDesc{};
#ifdef WEBGPU_BACKEND_WGPU
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    shaderCodeDesc.code = shaderSource.c_str();
#else
    shaderCodeDesc.source = shaderSource;
#endif
    shaderDesc.hintCount = 0;
    shaderDesc.hints = nullptr;
    shaderDesc.nextInChain = &shaderCodeDesc.chain;

    return device.createShaderModule(shaderDesc);
}

/*
 * Util function to get a WebGPU adapter
 */
/* WGPUAdapter request_adapter(WGPUInstance instance,
                            WGPURequestAdapterOptions const *options) {
    struct UserData {
        WGPUAdapter adapter{nullptr};
        bool requestEnded{false};
    };
    UserData userData;

    auto onAdapterRequestEnded = [](WGPURequestAdapterStatus status,
                                    WGPUAdapter adapter, char const
*message, void *ptrUserData) { UserData &userData =
*reinterpret_cast<UserData *>(ptrUserData); if (status ==
WGPURequestAdapterStatus_Success) { std::cout << "Successfully found
adapter: " << adapter << std::endl; userData.adapter = adapter; } else {
            std::cout << "Could NOT get WebGPU adapter: " << message
                      << std::endl;
        }
        userData.requestEnded = true;
    };

    wgpuInstanceRequestAdapter(instance, options, onAdapterRequestEnded,
                               (void *)&userData);

    assert(userData.requestEnded);

    return userData.adapter;
} */

/* void inspect_adapter(WGPUAdapter adapter) {
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
                  << limits.limits.maxStorageBuffersPerShaderStage <<
std::endl; std::cout << " - maxStorageTexturesPerShaderStage: "
                  << limits.limits.maxStorageTexturesPerShaderStage
                  << std::endl;
        std::cout << " - maxUniformBuffersPerShaderStage: "
                  << limits.limits.maxUniformBuffersPerShaderStage <<
std::endl; std::cout << " - maxUniformBufferBindingSize: "
                  << limits.limits.maxUniformBufferBindingSize << std::endl;
        std::cout << " - maxStorageBufferBindingSize: "
                  << limits.limits.maxStorageBufferBindingSize << std::endl;
        std::cout << " - minUniformBufferOffsetAlignment: "
                  << limits.limits.minUniformBufferOffsetAlignment <<
std::endl; std::cout << " - minStorageBufferOffsetAlignment: "
                  << limits.limits.minStorageBufferOffsetAlignment <<
std::endl; std::cout << " - maxVertexBuffers: " <<
limits.limits.maxVertexBuffers
                  << std::endl;
        std::cout << " - maxVertexAttributes: "
                  << limits.limits.maxVertexAttributes << std::endl;
        std::cout << " - maxVertexBufferArrayStride: "
                  << limits.limits.maxVertexBufferArrayStride << std::endl;
        std::cout << " - maxInterStageShaderComponents: "
                  << limits.limits.maxInterStageShaderComponents <<
std::endl; std::cout << " - maxComputeWorkgroupStorageSize: "
                  << limits.limits.maxComputeWorkgroupStorageSize <<
std::endl; std::cout << " - maxComputeInvocationsPerWorkgroup: "
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
        std::cout << " - driverDescription: " <<
properties.driverDescription
                  << std::endl;
    }
    std::cout << " - adapterType: " << properties.adapterType << std::endl;
    std::cout << " - backendType: " << properties.backendType << std::endl;
} */

/* WGPUDevice requestDevice(WGPUAdapter adapter,
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
} */

/* void inspectDevice(WGPUDevice device) {
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
                  << limits.limits.maxStorageBuffersPerShaderStage <<
std::endl; std::cout << " - maxStorageTexturesPerShaderStage: "
                  << limits.limits.maxStorageTexturesPerShaderStage
                  << std::endl;
        std::cout << " - maxUniformBuffersPerShaderStage: "
                  << limits.limits.maxUniformBuffersPerShaderStage <<
std::endl; std::cout << " - maxUniformBufferBindingSize: "
                  << limits.limits.maxUniformBufferBindingSize << std::endl;
        std::cout << " - maxStorageBufferBindingSize: "
                  << limits.limits.maxStorageBufferBindingSize << std::endl;
        std::cout << " - minUniformBufferOffsetAlignment: "
                  << limits.limits.minUniformBufferOffsetAlignment <<
std::endl; std::cout << " - minStorageBufferOffsetAlignment: "
                  << limits.limits.minStorageBufferOffsetAlignment <<
std::endl; std::cout << " - maxVertexBuffers: " <<
limits.limits.maxVertexBuffers
                  << std::endl;
        std::cout << " - maxVertexAttributes: "
                  << limits.limits.maxVertexAttributes << std::endl;
        std::cout << " - maxVertexBufferArrayStride: "
                  << limits.limits.maxVertexBufferArrayStride << std::endl;
        std::cout << " - maxInterStageShaderComponents: "
                  << limits.limits.maxInterStageShaderComponents <<
std::endl; std::cout << " - maxComputeWorkgroupStorageSize: "
                  << limits.limits.maxComputeWorkgroupStorageSize <<
std::endl; std::cout << " - maxComputeInvocationsPerWorkgroup: "
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
} */

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

    SupportedLimits supportedLimits;
    adapter.getLimits(&supportedLimits);
    std::cout << "adapter.maxVertexAttributes: "
              << supportedLimits.limits.maxVertexAttributes << std::endl;
    RequiredLimits requiredLimits = Default;
    requiredLimits.limits.maxVertexAttributes = 2;
    requiredLimits.limits.maxVertexBuffers = 1;
    // max size of buffer is 6 vertices of 2 float each
    requiredLimits.limits.maxBufferSize = 15 * 5 * sizeof(float);
    // max stride between 2 consecutive vertices in the vertex buffer
    // what is a stride??
    requiredLimits.limits.maxVertexBufferArrayStride = 5 * sizeof(float);
    requiredLimits.limits.minStorageBufferOffsetAlignment =
        supportedLimits.limits.minStorageBufferOffsetAlignment;
    requiredLimits.limits.minUniformBufferOffsetAlignment =
        supportedLimits.limits.minUniformBufferOffsetAlignment;
    // max number of components that can be forwarded vertex -> fragment shader
    requiredLimits.limits.maxInterStageShaderComponents = 3;

    std::cout << "Requesting device..." << std::endl;

    DeviceDescriptor deviceDesc{};
    // deviceDesc.nextInChain = nullptr;
    // label is used in error message for debugging
    // currently only used by Dawn
    deviceDesc.label = "My Device";
    deviceDesc.requiredFeaturesCount = 0;
    deviceDesc.requiredLimits = &requiredLimits;
    deviceDesc.defaultQueue.label = "The default queue";

    Device device = adapter.requestDevice(deviceDesc);

    std::cout << "Got device: " << device << std::endl;

    adapter.getLimits(&supportedLimits);
    device.getLimits(&supportedLimits);

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

    // Get the queue - Our WebGPU device has a single queue
    // BUT in the future there might be multiple queues per device
    Queue queue = device.getQueue();
    std::cout << "Creating swapchain..." << std::endl;

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
    // TextureFormat swapChainFormat = TextureFormat::BGRA8Unorm;
#else
    TextureFormat swapChainFormat = TextureFormat::BGRA8Unorm;
#endif // WEBGPU_BACKEND_WGPU

    std::cout << "swapChainFormat: " << swapChainFormat
              << std::endl; // 23 or 24

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

    // create swap chain
    // A new swap chain will be needed when window is _resized_!!!
    // Do not try to resize it for now
    SwapChainDescriptor swapChainDesc{};
    // swapChainDesc.nextInChain = nullptr;
    swapChainDesc.width = 640;
    swapChainDesc.height = 480;
    // specify usage
    swapChainDesc.usage = TextureUsage::RenderAttachment;
    swapChainDesc.format = swapChainFormat;
    swapChainDesc.presentMode = PresentMode::Fifo;

    // create the swap chain
    SwapChain swapChain = device.createSwapChain(surface, swapChainDesc);
    std::cout << "Swapchain: " << swapChain << std::endl;

    // ShaderModuleWGSLDescriptor shaderCodeDesc;
    // shaderCodeDesc.chain.next = nullptr;
    // shaderCodeDesc.chain.sType = SType::ShaderModuleWGSLDescriptor;
    // ShaderModuleDescriptor shaderDesc{};
    // shaderDesc.nextInChain = &shaderCodeDesc.chain;

    // ShaderModule shaderModule = device.createShaderModule(shaderDesc);
    std::cout << "Creating shader module..." << std::endl;
    ShaderModule shaderModule =
        loadShaderModule(RESOURCE_DIR "/shader.wgsl", device);
    std::cout << "Shader module: " << shaderModule << std::endl;

    // Vertex fetch
    VertexBufferLayout vertexBufferLayout;
    // Build vertex buffer layout
    // VertexAttribute vertexAttrib;
    std::vector<VertexAttribute> vertexAttribs(2);

    // position
    vertexAttribs[0].shaderLocation = 0;
    vertexAttribs[0].format = VertexFormat::Float32x2;
    vertexAttribs[0].offset = 0;
    // color
    vertexAttribs[1].shaderLocation = 1;
    vertexAttribs[1].format = VertexFormat::Float32x3;
    vertexAttribs[1].offset = 2 * sizeof(float);

    /* // === Per Attribute ==
    // @location(...)
    vertexAttrib.shaderLocation = 0;
    // vec2f in shader
    vertexAttrib.format = VertexFormat::Float32x2;
    // index of the first element
    vertexAttrib.offset = 0; */
    vertexBufferLayout.attributeCount =
        static_cast<uint32_t>(vertexAttribs.size());
    vertexBufferLayout.attributes = vertexAttribs.data();
    vertexBufferLayout.arrayStride = 5 * sizeof(float);
    // `VertexStepMode::Vertex` - each vertex corresponds to a different value
    // from the buffer if set to `Instance`, each value is shared by all
    // vertices of the same instance of the shape
    vertexBufferLayout.stepMode = VertexStepMode::Vertex;

    RenderPipelineDescriptor pipelineDesc{};
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;

    // vertex shader
    pipelineDesc.vertex.module = shaderModule;
    pipelineDesc.vertex.entryPoint = "vs_main"; // ???
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;

    // primitive pipeline state
    // each sequence of 3 vertices is a **triangle**
    pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
    // the order in which vertices should be connected
    // when not specified, connected _sequentially_
    pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
    // face orientation
    // assuming that when looking from the front of the face, its corner
    // vertices are enumerated in the counter-clockwise (CCW) order
    pipelineDesc.primitive.frontFace = FrontFace::CCW;
    // cull (hide) faces pointer away from us (often used for optimization)
    // no cull for now - the face orientation does not matter
    // _usually_ set to `Front`
    pipelineDesc.primitive.cullMode = CullMode::None;

    // Fragment shader
    FragmentState fragmentState{};
    // configure the blend stage here
    // fragment stage is _optional_
    // pipelineDesc.fragment is potentially nullptr
    pipelineDesc.fragment = &fragmentState;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;

    BlendState blendState{};
    // usual alpha blending for the color
    blendState.color.srcFactor = BlendFactor::SrcAlpha;
    blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = BlendOperation::Add;

    // leave target alpha untouched
    blendState.alpha.srcFactor = BlendFactor::Zero;
    blendState.alpha.dstFactor = BlendFactor::One;
    blendState.alpha.operation = BlendOperation::Add;

    ColorTargetState colorTarget{};
    colorTarget.format = swapChainFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = ColorWriteMask::All;

    // Only one target,
    // because our render pass has only one output color attachment
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;

    // depth and stencil tests are not used here
    pipelineDesc.depthStencil = nullptr;

    // Turn off multi-sampling
    // samples per pixel
    pipelineDesc.multisample.count = 1;
    // default value for the mask
    // *ALL* bits _ON_
    pipelineDesc.multisample.mask = ~0u;
    // default value
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    // not using any resources for now
    // asks the backend to figure out the layout by itself by inspecting the
    // shader
    pipelineDesc.layout = nullptr;

    // describe render pipeline
    RenderPipeline pipeline = device.createRenderPipeline(pipelineDesc);
    std::cout << "Render pipeline: " << pipeline << std::endl;

    // Vertex Buffer
    // 2 floats per vertex , one for x and one for y
    // *layout* tells GPU how to interpret this
    // std::vector<float> vertexData{-0.5, -0.5, +0.5, -0.5, +0.0, +0.5};
    // clang-format off
    /* std::vector<float> pointData = {
        // x0,   y0,  r0,  g0,  b0
        -0.5,   -0.5, 1.0, 0.0, 0.0, 
        // x1,   y1,  r1,  g1,  b1
        +0.5,   -0.5, 0.0, 1.0, 0.0,
        +0.5,   +0.5, 0.0, 0.0, 1.0,
        -0.5,   +0.5, 1.0, 1.0, 1.0,
    }; */

    // clang-format on
    // int vertexCount = static_cast<int>(vertexData.size() / 5);

    std::vector<float> pointData;
    std::vector<uint16_t> indexData;
    std::cout << "DIR: " << RESOURCE_DIR << std::endl;
    if (!loadGeometry(RESOURCE_DIR "/webgpu.txt", pointData, indexData)) {
        std::cerr << "Could not load geometry!" << std::endl;
        return 1;
    }

    // Create GPU vertex buffer
    BufferDescriptor bufferDesc;
    bufferDesc.size = pointData.size() * sizeof(float);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex;
    bufferDesc.mappedAtCreation = false;
    Buffer vertexBuffer = device.createBuffer(bufferDesc);

    // Upload geometry data to buffer
    queue.writeBuffer(vertexBuffer, 0, pointData.data(), bufferDesc.size);

    // Index buffer
    // A list of indices referencing positions in `pointData`
    // clang-format off
    /* std::vector<uint16_t> indexData = {
        0, 1, 2, // Triangle #0
        0, 2, 3  // Triangle #1
    }; */

    // clang-format on
    int indexCount = static_cast<int>(indexData.size());

    // Create index buffer
    bufferDesc.size = indexData.size() * sizeof(float);
    bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
    bufferDesc.mappedAtCreation = false;
    Buffer indexBuffer = device.createBuffer(bufferDesc);
    queue.writeBuffer(indexBuffer, 0, indexData.data(), bufferDesc.size);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Get target texture view
        // returns TextView
        // gives us a restricted access to the actual texture object allocated
        // by the swap chain
        TextureView nextTexture = swapChain.getCurrentTextureView();
        std::cout << "nextTexture: " << nextTexture << std::endl;

        // Getting the texture view _MAY FAIL_
        // especially when the window has been _resized_ thus the target surface
        // changed
        if (!nextTexture) {
            std::cerr << "CANNOT acquire next swap chain texture!" << std::endl;
            break;
        }

        CommandEncoderDescriptor commandEncoderDesc{};
        // commandEncoderDesc.nextInChain = nullptr;
        commandEncoderDesc.label = "CommandEncoder";
        CommandEncoder encoder =
            device.createCommandEncoder(commandEncoderDesc);

        // Draw
        // Create Command Encoder
        // Encode Render Pass
        // Finish encoding and submit
        RenderPassDescriptor renderPassDesc{};

        RenderPassColorAttachment renderPassColorAttachment{};
        // the texture view it must draw in
        // in advanced pipelines it's common to draw on intermediate textures,
        // which are then fed to e.g. post-process passes
        renderPassColorAttachment.view = nextTexture;

        renderPassColorAttachment.resolveTarget = nullptr;

        // loadOp indicates the load operation to perform on view _prior_ to
        // executing the render pass here sets it to default value
        renderPassColorAttachment.loadOp = LoadOp::Clear;

        // storeOp indicates the operation to perform on view after executing
        // the render pass can be either stored/discarded
        renderPassColorAttachment.storeOp = StoreOp::Store;

        // clearValue is the value to clear screen with
        // The 4 values to put:
        //   - red
        //   - green
        //   - blue
        //   - alpha channels
        // on a scale of 0.0 - 1.0
        renderPassColorAttachment.clearValue = Color{0.9, 0.1, 0.2, 1.0};

        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = &renderPassColorAttachment;

        renderPassDesc.depthStencilAttachment = nullptr;

        // for potential performance measurements
        renderPassDesc.timestampWriteCount = 0;
        renderPassDesc.timestampWrites = nullptr;

        // renderPassDesc.nextInChain = nullptr;

        RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

        // In its overall pipeline, draw a triangle
        renderPass.setPipeline(pipeline);
        renderPass.setVertexBuffer(0, vertexBuffer, 0,
                                   pointData.size() * sizeof(float));
        renderPass.setIndexBuffer(indexBuffer, IndexFormat::Uint16, 0,
                                  indexData.size() * sizeof(uint16_t));
        // Draw 1 instance of a 3-vertice shape
        // renderPass.draw(vertexCount, 1, 0, 0);
        renderPass.drawIndexed(indexCount, 1, 0, 0, 0);
        // directly end the pass without any other command
        renderPass.end();

        // Destroy texture view
        // the texture view is used _only for a single frame_
        // wgpuTextureViewDrop(nextTexture);
        nextTexture.drop();

        // generates the command from the encoder
        CommandBufferDescriptor cmdBufferDescriptor{};
        // cmdBufferDescriptor.nextInChain = nullptr;
        cmdBufferDescriptor.label = "CommandBuffer";
        // this operation destroys `encoder`
        CommandBuffer command = encoder.finish(cmdBufferDescriptor);

#ifdef WEBGPU_BACKEND_DAWN
        wgpuCommandEncoderRelease(encoder);
        wgpuCommandBufferRelease(command);
#endif // WEBGPU_BACKEND_DAWN

        // submit the command queue
        // destroys `command` buffer
        queue.submit(command);

        // Present swap chain
        // once the texture is filled in and view released,
        // tell the swap chain to present the next texture
        swapChain.present();
    }

    wgpuSwapChainDrop(swapChain);
    wgpuDeviceDrop(device);
    // Destroy the adapter
    // This function is wgpu-native specific
    wgpuAdapterDrop(adapter);
    // 5. clean up the WebGPU instance
    wgpuInstanceDrop(instance);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
