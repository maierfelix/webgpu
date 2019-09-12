{
  "variables": {
    "root": "../../..",
    "platform": "<(OS)",
    "build": "<@(module_root_dir)/build",
    "release": "<(build)/Release",
    "dawn": "C:/Users/User/Documents/GitHub/dawn"
  },
  "conditions": [
    [ "platform == 'win'",   { "variables": { "platform": "win" } } ],
    [ "platform == 'linux'", { "variables": { "platform": "linux" } } ],
    [ "platform == 'mac'",   { "variables": { "platform": "darwin" } } ]
  ],
  "targets": [
    {
      "target_name": "action_after_build",
      "type": "none",
      "conditions": []
    },
    {
      "sources": [
        "src/index.cpp"
      ],
      "conditions": [
        [
          "OS=='win'",
          {
            "target_name": "addon-win32",
            "cflags": [
              "-stdlib=libstdc++"
            ],
            "include_dirs": [
              "<!@(node -p \"require('node-addon-api').include\")",
              "<(root)/lib/include",
              "<(dawn)/src/include",
              "<(dawn)/out/Shared2/gen"
            ],
            "library_dirs": [
              "<(root)/lib/<(platform)/<(target_arch)/GLFW",
              "<(build)/"
            ],
            "link_settings": {
              "libraries": [
                "-lglfw3dll.lib",
                "-llibc++.dll.lib",
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
              "/bigobj",
              "DAWN_ENABLE_BACKEND_D3D12"
            ],
            "msvs_settings": {
              "VCCLCompilerTool": {
                "FavorSizeOrSpeed": 1,
                "StringPooling": "true",
                "Optimization": 2,
                "WarningLevel": 3,
                "AdditionalOptions": ["/MP /EHsc /wd4458 /wd4996 /wd4702 /wd4189"],
                "ExceptionHandling": 1
              },
              "VCLibrarianTool": {
                "AdditionalOptions" : ["/NODEFAULTLIB:MSVCRT"]
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
