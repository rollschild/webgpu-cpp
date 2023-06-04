# WebGPU in C++

## The Adapter

- In order to dialog with the GPU, the first thing is to get a WebGPU \***\*adapter\*\***
  - the main entry point of the lib
- Host system may expose _multiple_ adapters if it has multiple implementations of the WebGPU backend
- Example JS code:

```javascript
const adapter = await navigator.gpu.requestAdapter(options);
```

which is equivalent to:

```javascript
function onAdapterRequestSuccess(adapter) {
  // ...
}
navigator.gpu.requestAdapter(options).then(onAdapterRequestSuccess);
```

- Convention of `webgpu.h`:
  - names of the procedure always follow:
    ```c++
    wgpuSomethingSomeAction(something, ...)
    ```
- `glfwWindowHint()` function - a way to pass optional args to `glfwCreateWindow`
- The adapter object provides information about:
  - the underlying implementation and hardware, and
  - what it is able/not able to do

## The Device

- \***\*Device\*\*** represents a **context** of use of the API
  - owning all the objects that are created:
    - geometry,
    - textures, etc
- The **adapter** is used to access the capabilities of the customer's hardware, which are used to select the behavior of the application among very different code paths
- Once code path is chosen, a **device** is created with the chosen capabilities
  - only the capabilities selected for this device are then followed for the rest of the application

## The Command Queue

- There are _two_ processors running _simultaneously_
  - CPU - the **host**
  - GPU - the **device**
- Rules:
  - Code written is run on **CPU**; some of the code _triggers_ operations on GPU
    - the _only_ exception: \***\*shaders\*\*** - actually run on GPU
  - Processors are _far away_ - communication takes time
- Commands meant to the GPU are batched and fired through a \***\*command queue\*\***
- GPU consumes the command queue whenever it's ready
- Code written (in C++) for the CPU-side lives in the \***\*content timeline\*\***
- The other side of the command queue is the \***\*queue timeline\*\***, running on GPU
- \***\*Device timeline\*\***
  - associated with the GPU device operations
  - synchronous operations
  - the agent waits for an immediate answer
- Other graphics API allow for multiple queues per device
  - might be supported by future versions of WebGPU
- Idiom:
  - WebGPU is a _C_ API
  - whenever an array is passed to the API, its size should be known
- Three different ways to submit work to the queue:
  - `wgpuQueueSubmit` - _only_ sends commands
  - `wgpuQueueWriteBuffer` - sends memory from CPU memory (RAM) to the GPU one (VRAM)
  - `wgpuQueueWriteTexture` - sends memory from CPU memory (RAM) to the GPU one (VRAM)
- the `WGPUCommandBuffer` object _CANNOT_ be manually created
  - to build it, use \***\*command encoder\*\***

## Swap Chain

### Drawing Process

- The render pipeline does _not_ draw directly on the texture currently displayed
- A typical pipeline draws to an off-screen texture, which replaces the currently displayed one _only once_ it's complete
  - the texture is \***\*presented\*\*** to the surface
- Drawing takes a _different time_ than the frame rate required by the application
  - GPU may have to wait until the next frame is needed
  - there might be _more than one_ off-screen waiting in the queue to be presented
- The off-screen textures are _reused_ as much as possible
  - as soon as a new texture is presented, previous ones can be reused as a target for the next time
  - the whole texture swapping mechanism is implemented by \***\*Swap Chain\*\***

### Creation

- create the swap chain descriptor
- swap chain allocates textures
  - textures are allocated for a _specific usage_, which dictates the way the GPU organizes its memory
- Specify the present mode - which texture from the waiting queue must be presented at each frame
  - `Immediate`
  - `Mailbox`
  - `Fifo`
- Swap Chain is _NOT_ provided in the JavaScript API
  - taken care of by the browser

## Texture View

- Swap Chain provides us with the texture where to draw the next frame

## Render Pass

### Render Pass Encoder

- Like any GPU-side operation,
  - trigger drawing operations from the **command queue**
- A render pass leverages 3D rendering circuits of the GPU to draw content into one or multiple textures
  - Important to tell _which textures are the target_ of this process
  - a.k.a. \***\*attachments\*\*** of the render pass
  - number of attachments is variable

### Misc

- A special type of attachment, which is a _single_ attachment potentially containing two channels
  - **depth**
  - **stencil**
- When measuring performance of a render pass, it's _NOT_ possible to use CPU-side timings since the commands are _NOT_ executed synchronously
