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

## Render Pipeline

- GPU processes shapes through a _predefined_ pipeline
- the pipeline itself is _always_ the same
  - generally burnt into the physical arch of the hardware
  - but we can configure it

### Vertex Pipeline State

- Both \***\*vertex fetch\*\*** and \***\*vertex shader\*\*** stages are configured through the **vertex state** architecture
- The render pipeline first fetches vertex attributes from some buffers
- Then each vertex is processed by a custom \***\*vertex shader\*\***
- Shader - the combination of:
  - a shader module
  - an entry point
  - an array of value assignments for the **constants** of the shader

### Primitive Pipeline State

- \***\*primitive state\*\*** configures the \***\*primitive assembly\*\*** and \***\*rasterization\*\*** stages
- \***\*rasterization\*\***
  - heart of 3D rendering
  - transforms a **primitive** (point, line, or triangle) into a series of **fragments**, that
    - correspond to the pixels covered by the primitive
  - interpolates any extra attribute output by the vertex shader
- **Fragment**
  - the projection of a given primitive on a given pixel
- the **primitive assembly configuration** consists of stating how the array of vertices fetched earlier must be connected to form
  - a point cloud
  - a wire frame, or
  - a triangle

### Fragment Shader

- Once a primitive has been turned into fragments
  - the **fragment shader** stage is invoke for _each one of_ them

### Stencil/Depth State

- **Depth test**
  - discard fragments that are _behind_ other fragments associated to the same pixel
- When primitives overlap, multiple fragments are emitted for the same pixel
- Fragments have a **depth** info
- **Stencil test**
  - another fragment discarding mechanism
  - hide fragments based on previously rendered primitives

### Blending

- Takes each fragment's color and paints it onto the target color attachment
- Must specify what _format_ the colors are to be stored in the final attachment

#### Blending Equation

- General form:
  - `rgb = srcFactor * rgb_s [operation] dstFactor * rgb_d`
- **Alpha blending**
  - `rgb = a_s * rgb_s + (1 - a_s) * rgb_d`
  - where a pixel can be represented `(r, g, b, a)`

### Multi-Sampling

- Can split pixels into sub-elements, \***\*samples\*\***
- fragment is associated to a sample
- value of a pixel is computed by averaging its associated samples
  - **multi-sampling**
    - used for anti-aliasing

## Shaders

- **Shader module**
  - a dynamic lib
  - talks in binary language of GPU
- The application is distributed with the **source code** of the shaders,
  - compiled on the fly when initializing the application

### Shader Code

- \***\*WGSL\*\*** - _official_ shader language used by WebGPU
  - \***\*WebGPU Shading Language\*\***
- Shader languages natively support **vector** and **matrix** types up to size **4**
  - `vec2f` - vector of 2 `float`s
    - `vec2<f32>`
  - `vec4<u32>` - vector of 4 unsigned integers
    - `vec4u`
- \***\*Attributes\*\***
  - starting with `@`
- For more flexibility, shader code should be loaded from a file
- `nextInChain`
  - entry point of WebGPU's **extension mechanism**
  - either
    - Null, or
    - pointing to a structure of type `WGPUChainedStruct`
- `WGPUChainedStruct` - has two fields
  - `next` - recursively points to the next
  - `SType` - an enum - in which struct this chain element can be cast
- To create a shader module from WGSL code, use `ShaderModuleWGSLDescripter` SType

### Pipeline Layout

- shaders might need to _access input/output resources_
  - buffers and/or textures
  - made available to the pipeline by configuring a memory layout

## Input Geometry

### Buffer

- \***\*Buffer\*\*** - a chunk of memory allocated in the VRAM (GPU's memory)
  - `new` or `malloc` for the GPU
- We _must_ state hints about of use of the memory _upon creation_
- a GPU buffer is **mapped** when it's connected to a specific part of the CPU-side RAM
  - driver then automatically synch its content (reading/writing)
- Remember to free the buffers!
  ```c++
  buffer.destroy();
  wgpuBufferDrop(buffer); // wgpuBufferRelease
  ```
  - `destroy()` frees the GPU memory that was allocated for the buffer
  - `drop` frees the driver/backend side object

### Writing to Buffer

- uploading data from CPU-size memory to GPU-side (VRAM) _takes time_
- when `writeBuffer()` returns, data transfer _may not_ have finished yet BUT guaranteed that
  - memory from the address just passed can be freed
    - back-end maintains its own CPU-side copy of the buffer during transfer
  - commands submitted in the queue _after_ `writeBuffer()` will _NOT_ be executed _before_ data transfer is completed

### Reading from Buffer

- _CANNOT_ just use the command queue to read memory back from GPU - this is a "fire and forget" queue
  - functions do _NOT_ return a value since they are run on a _different timeline_
- Use _async_ operations
  - `wgpuBufferMapAsync` (or `buffer.mapAsync`)
  - maps GPU buffer into CPU memory
  - whenever it's ready it executes the callback function provided

### Asynchronous Polling

- There is _NO_ hidden process executed by the WebGPU lib to check that the async operation is ready
- Backend checks for ongoing async operations _only when_ we call another op

## Vertext Attributes

### Shader

- We do _NOT_ control how `vs_main` is invoked
  - it's controlled by the fixed part of the render pipeline
- In the code below, arg `in_vertex_index` must be populated by the vertex fetch stage with the index of the current index
  ```wgsl
    @vertex
    fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
    }
  ```
  - `@builtin(vertex_index)` is a **WGSL Attribute**
- To create custom input (instead of built-in):

  1. Create a **buffer** to store the value of the input for each vertex - stored on GPU side
  2. Tell render pipeline how to interpret the raw buffer data when fetching an entry for each vertex - the vertex buffer **layout**
  3. Set vertex buffer in the render pass before the draw call

- **Attribute**
  - WGSL attribute: tokens like `@something` in WGSL code
  - **vertex attribute**: an input of the vertex shader
- Number of vertex attributes available for the device may vary if not specified

### Vertex Buffer Layout

- the **vertex fetch** stage
  - provide data from the vertex buffer to the vertex shader
- The same vertex buffer can contain multiple vertex attributes
  - `maxVertexAttributes` and `maxVertexBuffers` limits are _different_
- **stride**
  - a common concept in buffer manipulation
  - designates the number of bytes between two consecutive elements
  - if positions are _contiguous_, stride is equal to the size of a `vec2f` - will change if more attributes are added in the same buffer

### Render Pass

- Connect the vertex buffer to the pipeline's vertex buffer layout when encoding the render pass

## Multiple Attributes

- Vertices can contani _more than_ just a position
  - color attribute

### Shader

- vertex attributes are _only_ provided in the vertex shader
  - HOWEVER, the **fragment shader** can receive what the vertex shader returns
- There is a **limit** on the number of components that can be forwarded vertex -> fragment shader

### Vertex Buffer Layout

- Multiple ways to feed multiple attributes to the vertex fetch stage

#### Option A: Interleaved Attributes

- Put in a _single_ buffer the values for _all_ attributes of the first vertex, then all values for the second vertex, and so on

#### Option B: Multiple Buffers

- Have different buffers for different attributes
- Need to change `requiredLimits.limits.maxVertexBuffers`
- This leads to multiple GPU buffers
  - multiple `queue.writeBuffer()`

## Index Buffer

- **Index buffer** - separate list of vertex attributes from the actual order in which they are connected
- Index data must be stored in GPU-side buffer
- Indices are _relative to_ the window's dimentions
  - fixed by multiplying one of the coordinates by the ratio of the window (640/480)
- `CAMKE_CURRENT_SOURCE_DIR`
- `CMAKE_BUILD_TYPE`
  - a built-in option of CMake
  - commonly used
  - set it to `Debug` to compile the program with **debugging symbols**
    - slower and heavier executable
  - set it to `Release` - no debugging safe-guard
    - faster and lightweight executable
- Color Space Issue
  - the colors expressed in a given space are _inerpreted differently_
  - **color space**: how are the 256 possible colors distributed along the _continuous_ range of light intensity
    - **linear**
    - **sRGB**
  - WebGPU assumes that colors output by the fragment shader are linear, so it does an extra linear to sRGB conversion
- ****Gamma Correction**** (or ****Tone Mapping****) - conversion from linear to non-linear color scale
