# WebGPU in C++

This is a learning project for me to learn how to program WebGPU, in C++.

## Wayland

To build for wayland, there are a few caveats/workarounds.

- Try to use `glfw-wayland` instead of `glfw`
- Under `glfw3webgpu/`, modify the macro in `./glfw3webgpu/glfw3webgpu.c`:
  - `#define WGPU_TARGET WGPU_TARGET_LINUX_WAYLAND`

## Build

- To build in DEV mode:

```console
$ cmake -B build-dev -DDEV_MODE=ON -DCMAKE_BUILD_TYPE=Debug
$ make -C build-dev
$ ./build-dev/src/App
```

- To build in RELEASE mode:

```console
$ cmake -B build-release -DDEV_MODE=OFF -DCMAKE_BUILD_TYPE=Release
$ make -C build-release
$ ./build-release/src/App
```

-
