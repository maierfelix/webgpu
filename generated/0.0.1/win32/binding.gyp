{
  "variables": {
    "root": "../../..",
    "platform": "<(OS)",
    "build": "<@(module_root_dir)/build",
    "release": "<(build)/Release",
    "dawn": "C:/Users/User/Documents/GitHub/dawn",
    "vkSDK": "C:/VulkanSDK/1.1.126.0"
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
      "sources": [
        "src/*.cpp"
      ],
      "conditions": [
        [
          "OS=='win'",
          {
            "target_name": "addon-win32",
            "cflags!": [
              "-fno-exceptions"
            ],
            "cflags_cc!": [
              "-fno-exceptions"
            ],
            "include_dirs": [
              "<!@(node -p \"require('node-addon-api').include\")",
              "<(vkSDK)/include",
              "<(root)/lib/include",
              "<(dawn)/src/include",
              "<(dawn)/out/Shared/gen",
              "<(dawn)/third_party/shaderc/libshaderc/include",
              "<(dawn)/third_party/shaderc/libshaderc/src/shaderc.cc",
              "<(dawn)/third_party/shaderc/libshaderc/src/shaderc_private.h"
            ],
            "library_dirs": [
              "<(root)/lib/<(platform)/<(target_arch)/GLFW",
              "<(build)/",
              "<(vkSDK)/lib"
            ],
            "link_settings": {
              "libraries": [
                "-lvulkan-1.lib",
                "-lglfw3dll.lib",
                "-llibdawn.dll.lib",
                "-llibdawn_native.dll.lib",
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
              "DAWN_ENABLE_BACKEND_VULKAN",
              "_GLFW_WIN32",
              "DAWN_NATIVE_SHARED_LIBRARY",
              "DAWN_WIRE_SHARED_LIBRARY",
              "VK_USE_PLATFORM_WIN32_KHR",
              "DAWN_SHARED_LIBRARY",
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
          }
        ]
      ]
    }
  ]
}
