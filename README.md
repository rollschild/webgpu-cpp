# WebGPU in C++

This is a learning project for me to learn how to program WebGPU, in C++.

## Wayland

To build for wayland, there are a few caveats/workarounds.

- Try to use `glfw-wayland` instead of `glfw`
- Under `glfw3webgpu/`, modify the macro in `./glfw3webgpu/glfw3webgpu.c`:
  - `#define WGPU_TARGET WGPU_TARGET_LINUX_WAYLAND`
