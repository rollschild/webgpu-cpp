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
