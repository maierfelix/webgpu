{
  "variables": {
    "root": "../../..",
    "platform": "<(OS)",
    "build": "<@(module_root_dir)/build",
    "release": "<(build)/Release",
    "dawn": "/home/dcerisano/git/dawn-ray-tracing"
  },
  "conditions": [
    [ "platform == 'win'",   { "variables": { "platform": "win" } } ],
    [ "platform == 'linux'", { "variables": { "platform": "linux" } } ],
    [ "platform == 'mac'",   { "variables": { "platform": "darwin" } } ]
  ],
  "make_global_settings": [
    ['CXX','/usr/bin/clang++'],
    ['LINK','/usr/bin/clang++'],
  ],
  "targets": [
    {
      "target_name": "action_after_build",
      "type": "none",
      "conditions": []
    },
    {
      "conditions": [
        [
          "OS=='linux'",
          {
            "sources": [
              "src/index.cpp",
              "src/BackendBinding.cpp",
              "src/DescriptorDecoder.cpp",
              "src/GPU.cpp",
              "src/GPUAdapter.cpp",
              "src/GPUBindGroup.cpp",
              "src/GPUBindGroupLayout.cpp",
              "src/GPUBuffer.cpp",
              "src/GPUCanvasContext.cpp",
              "src/GPUCommandBuffer.cpp",
              "src/GPUCommandEncoder.cpp",
              "src/GPUComputePassEncoder.cpp",
              "src/GPUComputePipeline.cpp",
              "src/GPUDevice.cpp",
              "src/GPUFence.cpp",
              "src/GPUPipelineLayout.cpp",
              "src/GPUQueue.cpp",
              "src/GPURayTracingAccelerationContainer.cpp",
              "src/GPURayTracingPassEncoder.cpp",
              "src/GPURayTracingPipeline.cpp",
              "src/GPURayTracingShaderBindingTable.cpp",
              "src/GPURenderBundle.cpp",
              "src/GPURenderBundleEncoder.cpp",
              "src/GPURenderPassEncoder.cpp",
              "src/GPURenderPipeline.cpp",
              "src/GPUSampler.cpp",
              "src/GPUShaderModule.cpp",
              "src/GPUSwapChain.cpp",
              "src/GPUTexture.cpp",
              "src/GPUTextureView.cpp",
              "src/NullBinding.cpp",
              "src/VulkanBinding.cpp",
              "src/WebGPUWindow.cpp"
            ],
            "target_name": "addon-linux",
            "defines": [
              "DAWN_ENABLE_BACKEND_NULL",
              "DAWN_ENABLE_BACKEND_VULKAN",
              "DAWN_NATIVE_SHARED_LIBRARY",
              "DAWN_WIRE_SHARED_LIBRARY",
              "WGPU_SHARED_LIBRARY",
              "NAPI_CPP_EXCEPTIONS"
            ],
            "include_dirs": [
              "<!@(node -p \"require('node-addon-api').include\")",
              "<(dawn)/third_party/vulkan-headers/include",
              "<(root)/lib/include",
              "<(dawn)/src/include",
              "<(dawn)/out/Shared/gen/src/include"
            ],
            "cflags": [
              "-std=c++14",
              "-fexceptions",
              "-Wno-switch",
              "-Wno-unused",
              "-Wno-uninitialized"
            ],
            "cflags_cc": [
              "-std=c++14",
              "-fexceptions",
              "-Wno-switch",
              "-Wno-unused",
              "-Wno-uninitialized"
            ],
            "library_dirs": [
              "<(release)",
              "<(module_root_dir)/../../../lib/<(platform)/<(target_arch)",
              "<(module_root_dir)/../../../lib/<(platform)/<(target_arch)/GLFW"
            ],
            "libraries": ["-Wl,-rpath,<(release)",
              "-lglfw3",
              "-ldawn_native",
              "-ldawn_proc",
              "-ldawn_wire",
              "-lshaderc_spvc",
              "-lshaderc",
              "-lXrandr",
              "-lXi",
              "-lX11",
              "-lXxf86vm",
              "-lXinerama",
              "-lXcursor",
              "-ldl",
              "-pthread"  
            ]
          },
          "OS=='win'",
          {
            "sources": [
              "src/*.cpp"
            ],
            "target_name": "addon-win32",
            "cflags!": [
              "-fno-exceptions"
            ],
            "cflags_cc!": [
              "-fno-exceptions"
            ],
            "include_dirs": [
              "<!@(node -p \"require('node-addon-api').include\")",
              "<(dawn)/third_party/vulkan-headers/include",
              "<(root)/lib/include",
              "<(dawn)/src/include",
              "<(dawn)/out/Shared/gen/src/include",
              "<(dawn)/third_party/shaderc/libshaderc/include",
              "<(dawn)/third_party/shaderc/libshaderc/src/shaderc.cc",
              "<(dawn)/third_party/shaderc/libshaderc/src/shaderc_private.h"
            ],
            "library_dirs": [
              "<(root)/lib/<(platform)/<(target_arch)/GLFW",
              "<(build)/"
            ],
            "link_settings": {
              "libraries": [
                "-lglfw3dll.lib",
                "-llibdawn_native.dll.lib",
                "-llibdawn_proc.dll.lib",
                "-llibdawn_wire.dll.lib",
                "-llibshaderc.dll.lib",
                "-llibshaderc_spvc.dll.lib"
              ]
            },
            "defines": [
              "WIN32_LEAN_AND_MEAN",
              "NOMINMAX",
              "_UNICODE",
              "UNICODE",
              "DAWN_ENABLE_BACKEND_D3D12",
              "DAWN_ENABLE_BACKEND_NULL",
              "DAWN_ENABLE_BACKEND_VULKAN",
              "DAWN_NATIVE_SHARED_LIBRARY",
              "DAWN_WIRE_SHARED_LIBRARY",
              "WGPU_SHARED_LIBRARY",
              "_GLFW_WIN32",
              "VK_USE_PLATFORM_WIN32_KHR",
              "NAPI_CPP_EXCEPTIONS"
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "AdditionalOptions": ["/MP /EHsc"],
                "ExceptionHandling": 1
              },
              "VCLibrarianTool": {
                "AdditionalOptions" : []
              },
              "VCLinkerTool": {
                "AdditionalLibraryDirectories": [
                  "../@PROJECT_SOURCE_DIR@/lib/<(platform)/<(target_arch)",
                  "<(build)/"
                ]
              }
            }
          },
          "OS=='mac'",
          {
            "sources": [
              "src/index.cpp",
              "src/BackendBinding.cpp",
              "src/DescriptorDecoder.cpp",
              "src/GPU.cpp",
              "src/GPUAdapter.cpp",
              "src/GPUBindGroup.cpp",
              "src/GPUBindGroupLayout.cpp",
              "src/GPUBuffer.cpp",
              "src/GPUCanvasContext.cpp",
              "src/GPUCommandBuffer.cpp",
              "src/GPUCommandEncoder.cpp",
              "src/GPUComputePassEncoder.cpp",
              "src/GPUComputePipeline.cpp",
              "src/GPUDevice.cpp",
              "src/GPUFence.cpp",
              "src/GPUPipelineLayout.cpp",
              "src/GPUQueue.cpp",
              "src/GPURayTracingAccelerationContainer.cpp",
              "src/GPURayTracingPassEncoder.cpp",
              "src/GPURayTracingPipeline.cpp",
              "src/GPURayTracingShaderBindingTable.cpp",
              "src/GPURenderBundle.cpp",
              "src/GPURenderBundleEncoder.cpp",
              "src/GPURenderPassEncoder.cpp",
              "src/GPURenderPipeline.cpp",
              "src/GPUSampler.cpp",
              "src/GPUShaderModule.cpp",
              "src/GPUSwapChain.cpp",
              "src/GPUTexture.cpp",
              "src/GPUTextureView.cpp",
              "src/NullBinding.cpp",
              "src/WebGPUWindow.cpp",
              "src/MetalBinding.mm"
            ],
            "target_name": "addon-darwin",
            "defines": [
              "DAWN_ENABLE_BACKEND_NULL",
              "DAWN_ENABLE_BACKEND_METAL",
              "DAWN_NATIVE_SHARED_LIBRARY",
              "DAWN_WIRE_SHARED_LIBRARY",
              "WGPU_SHARED_LIBRARY",
              "NAPI_DISABLE_CPP_EXCEPTIONS"
            ],
            "include_dirs": [
              "<!@(node -p \"require('node-addon-api').include\")",
              "<(root)/lib/include",
              "<(dawn)/src/include",
              "<(dawn)/out/Shared/gen",
              "<(dawn)/out/Shared/gen/src/include",
              "<(dawn)/third_party/shaderc/libshaderc/include",
              "<(dawn)/third_party/shaderc/libshaderc/src/shaderc.cc",
              "<(dawn)/third_party/shaderc/libshaderc/src/shaderc_private.h"
            ],
            "libraries": [
              "<(release)/libdawn_native.dylib",
              "<(release)/libc++.dylib",
              "<(release)/libdawn_proc.dylib",
              "<(release)/libdawn_wire.dylib",
              "<(release)/libshaderc_spvc.dylib",
              "<(release)/libshaderc.dylib",
              "<(release)/../../<(root)/lib/<(platform)/<(target_arch)/GLFW/libglfw3.a"
            ],
            "xcode_settings": {
              "OTHER_CPLUSPLUSFLAGS": [
                "-std=c++14",
                "-stdlib=libc++"
              ],
              "OTHER_LDFLAGS": [
                "-Wl,-rpath,<(release)",
                "-framework Cocoa",
                "-framework IOKit",
                "-framework Metal",
                "-framework QuartzCore"
              ],
              "LIBRARY_SEARCH_PATHS": [
                "<(release)"
              ]
            }
          }
        ]
      ]
    }
  ]
}

















