import WebGPU from "../index.js";

Object.assign(global, WebGPU);

const writeVsSrc = `
  #version 450
  #pragma shader_stage(vertex)
  layout (location = 0) out vec2 vUV;
  void main(void) {
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0 + -1.0f, 0.0, 1.0);
    vUV = vec2(uv.x, 1.0 - uv.y);
  }
`;

const writeFsSrc = `
  #version 450
  #pragma shader_stage(fragment)
  layout (location = 0) in vec2 vUv;
  layout(set = 0, binding = 0, rgba8) uniform writeonly image2D image0;
  void main() {
    imageStore(image0, ivec2(vUv * vec2(640.0, 480.0)), vec4(vUv, 0.5, 0));
    discard;
  }
`;

const readVsSrc = `
  #version 450
  #pragma shader_stage(vertex)
  layout (location = 0) out vec2 vUV;
  void main(void) {
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0 + -1.0f, 0.0, 1.0);
    vUV = vec2(uv.x, 1.0 - uv.y);
  }
`;

const readFsSrc = `
  #version 450
  #pragma shader_stage(fragment)
  layout (location = 0) in vec2 vUv;
  layout (location = 0) out vec4 outColor;
  layout(set = 0, binding = 0, rgba8) uniform readonly image2D image0;
  void main() {
    outColor = imageLoad(image0, ivec2(vUv * vec2(640.0, 480.0)));
  }
`;

(async function main() {

  const window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU"
  });

  let adapter = await GPU.requestAdapter({ window });

  const device = await adapter.requestDevice();

  const queue = device.getQueue();

  const context = window.getContext("webgpu");

  const swapChainFormat = await context.getSwapChainPreferredFormat(device);

  const swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat
  });

  const storageTexture = device.createTexture({
    size: {
      width: window.width,
      height: window.height,
      depth: 1
    },
    arrayLayerCount: 1,
    mipLevelCount: 1,
    sampleCount: 1,
    dimension: "2d",
    format: "rgba8unorm",
    usage: GPUTextureUsage.STORAGE
  });

  const storageTextureView = storageTexture.createView({
    dimension: "2d",
    baseArrayLayer: 0,
    arrayLayerCount: 1,
    format: "rgba8unorm"
  });

  const writeBindGroupLayout = device.createBindGroupLayout({
    entries: [
      {
        binding: 0,
        visibility: GPUShaderStage.FRAGMENT,
        type: "writeonly-storage-texture",
        viewDimension: "2d",
        storageTextureFormat: "rgba8unorm"
      }
    ]
  });

  const writeBindGroup = device.createBindGroup({
    layout: writeBindGroupLayout,
    entries: [
      {
        binding: 0,
        textureView: storageTextureView,
        size: 0
      }
    ]
  });

  const writePipeline = device.createRenderPipeline({
    layout: device.createPipelineLayout({
      bindGroupLayouts: [writeBindGroupLayout]
    }),
    sampleCount: 1,
    vertexStage: {
      module: device.createShaderModule({ code: writeVsSrc }),
      entryPoint: "main"
    },
    fragmentStage: {
      module: device.createShaderModule({ code: writeFsSrc }),
      entryPoint: "main"
    },
    primitiveTopology: "triangle-list",
    vertexInput: {
      indexFormat: "uint32",
      buffers: []
    },
    rasterizationState: {
      frontFace: "CCW",
      cullMode: "none"
    },
    colorStates: [
      {
        format: swapChainFormat,
        alphaBlend: {},
        colorBlend: {}
      }
    ]
  });

  const readBindGroupLayout = device.createBindGroupLayout({
    entries: [
      {
        binding: 0,
        visibility: GPUShaderStage.FRAGMENT,
        type: "readonly-storage-texture",
        viewDimension: "2d",
        storageTextureFormat: "rgba8unorm"
      }
    ]
  });

  const readBindGroup = device.createBindGroup({
    layout: readBindGroupLayout,
    entries: [
      {
        binding: 0,
        textureView: storageTextureView,
        size: 0
      }
    ]
  });

  const readPipeline = device.createRenderPipeline({
    layout: device.createPipelineLayout({
      bindGroupLayouts: [readBindGroupLayout]
    }),
    sampleCount: 1,
    vertexStage: {
      module: device.createShaderModule({ code: readVsSrc }),
      entryPoint: "main"
    },
    fragmentStage: {
      module: device.createShaderModule({ code: readFsSrc }),
      entryPoint: "main"
    },
    primitiveTopology: "triangle-list",
    vertexInput: {
      indexFormat: "uint32",
      buffers: []
    },
    rasterizationState: {
      frontFace: "CCW",
      cullMode: "none"
    },
    colorStates: [
      {
        format: swapChainFormat,
        alphaBlend: {},
        colorBlend: {}
      }
    ]
  });

  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

    let commands = [];
    // storage texture write pass
    {
      const backBufferView = swapChain.getCurrentTextureView();
      const commandEncoder = device.createCommandEncoder({});
      const renderPass = commandEncoder.beginRenderPass({
        colorAttachments: [{
          clearColor: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
          loadOp: "clear",
          storeOp: "store",
          attachment: backBufferView
        }]
      });
      renderPass.setPipeline(writePipeline);
      renderPass.setBindGroup(0, writeBindGroup);
      renderPass.draw(3, 1, 0, 0);
      renderPass.endPass();
      commands.push(commandEncoder.finish());
    }

    // storage texture read pass
    {
      const backBufferView = swapChain.getCurrentTextureView();
      const commandEncoder = device.createCommandEncoder({});
      const renderPass = commandEncoder.beginRenderPass({
        colorAttachments: [{
          clearColor: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
          loadOp: "clear",
          storeOp: "store",
          attachment: backBufferView
        }]
      });
      renderPass.setPipeline(readPipeline);
      renderPass.setBindGroup(0, readBindGroup);
      renderPass.draw(3, 1, 0, 0);
      renderPass.endPass();
      commands.push(commandEncoder.finish());
    }

    queue.submit(commands);
    swapChain.present();
    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);

})();
