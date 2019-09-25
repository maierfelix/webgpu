# webgpu

This is a WebGPU API for native JavaScript, based on chromium's [Dawn Project](https://dawn.googlesource.com/dawn/)

## Installation

## Building

You have to build [dawn](https://dawn.googlesource.com/dawn) as a shared library.
After building dawn, create a file named `PATH_TO_DAWN` in this project's root, containing the path to dawn.

Now in order to generate the bindings and build this project, run:
````
npm run all --dawnversion=0.0.1
````
