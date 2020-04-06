# webgpu

This is a WebGPU API for native JavaScript, based on a [Fork](https://github.com/maierfelix/dawn-ray-tracing) of Chromium's [Dawn Project](https://dawn.googlesource.com/dawn/).

### Platforms:

This project comes with pre-built N-API binaries for the following platforms:

|       OS      |     Status    |
| ------------- | ------------- |
| <img src="https://i.imgur.com/FF3Ssp6.png" alt="" height="16px">  Windows       | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ✔ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|
| <img src="https://i.imgur.com/bkBCY7V.png" alt="" height="16px">  Linux         | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌✔ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|
| <img src="https://i.imgur.com/iPt4GHz.png" alt="" height="16px">  MacOS         | ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ X ‌‌ ‌‌ ‌‌ ‌‌ ‌‌ ‌‌|

## Building

You have to build [dawn](https://dawn.googlesource.com/dawn) as a shared library.
After building dawn, create a file named `PATH_TO_DAWN` in this project's root directory, containing the **absolute** path to dawn.

In case you have multiple python installations, you might want to use the `--script-executable` gn flag to instruct *gn* to use the python 2.x installation.

After you have generated and built dawn, you can now build this project by running:
````
npm run all --dawnversion=0.0.1
````

### Windows

Follow dawn's initial setup instructions, but instead of the standard build, do the following:

To generate the project as a shared library using MSVS:
````
gn gen out/Shared --ide=vs --target_cpu="x64" --args="is_component_build=true is_debug=false is_clang=false"
````
It's important that you build using MSVS and **not** clang, as otherwise you will potentially get linking errors.

To build the project run:
````
ninja -C out/Shared
````

In case python wasn't found:
 - Use `where python` to get the location of your python installation
 - Repoint it by running e.g. `npm config set python C:\depot_tools\python.bat`

### Linux 

Follow dawn's initial setup instructions, but instead of the standard build, do the following:

To generate the project as a shared library:
````
gn gen out/Shared --target_cpu="x64" --args="is_component_build=true is_debug=false is_clang=true"
````

To build the project run:
````
ninja -C out/Shared
````

If you do not have Vulkan installed, point to dawn Vulkan libraries for your system, eg:
````
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/your-path-to-dawn/build/linux/debian_sid_amd64-sysroot/usr/lib/x86_64-linux-gnu
````
OR minimally install `libvulkan1` package for your system, eg:
````
sudo apt install libvulkan1
````

### MacOS

Follow dawn's initial setup instructions, but instead of the standard build, do the following:

To generate the project as a shared library:
````
gn gen out/Shared --target_cpu="x64" --args="is_component_build=true is_debug=false is_clang=true"
````

To build the project run:
````
ninja -C out/Shared
````

## Examples
````
cd examples & cd ..
node --experimental-modules examples/interactive-triangle.mjs
````

## TODOs
 - Add CTS
 - Remove libshaderc from build?
