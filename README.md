# nwgpu
WebGPU API for JavaScript (Early Bird)

## Building

You have to build [dawn](https://dawn.googlesource.com/dawn) as a shared library.
After building dawn, create a file named `PATH_TO_DAWN` in this project's root, containing the path to dawn's root.

Now in order to generate and build *n-wgpu*, run:
````
npm run all --dawnversion=0.0.1
````

### Windows

Follow dawn's initial setup instructions, but instead of the standard build, do the following:

To generate the project as a shared library using MSVS:
````
gn gen out/Shared --ide=vs --target_cpu="x64" --args="is_component_build=true is_debug=false is_clang=false"
````
It's important that you build using MSVS, as otherwise you will potentially get linking errors.

To build the project run:
````
ninja -C out/Shared
````

### Linux // TODO
