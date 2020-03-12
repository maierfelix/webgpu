import WebGPU from "../index.js";

Object.assign(global, WebGPU);

const vsSrc = `
  #version 450
  #pragma shader_stage(vertex)
  const vec2 pos[3] = vec2[3](
    vec2(0.0f, 0.5f),
    vec2(-0.5f, -0.5f),
    vec2(0.5f, -0.5f)
  );
  void main() {
    gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
  }
`;

const fsSrc = `
  #version 450
  #pragma shader_stage(fragment)
  layout(location = 0) out vec4 outColor1;
  layout(location = 1) out vec4 outColor2;
  void main() {
    outColor1 = vec4(1.0, 0.0, 0.0, 1.0);
    outColor2 = vec4(0.0, 1.0, 0.0, 1.0);
  }
`;

const blitVsSrc = `
  #version 450
  #pragma shader_stage(vertex)
  layout (location = 0) out vec2 vUV;
  void main(void) {
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0 + -1.0f, 0.0, 1.0);
    vUV = vec2(uv.x, 1.0 - uv.y);
  }
`;

const blitFsSrc = `
  #version 450
  #pragma shader_stage(fragment)
  layout (location = 0) in vec2 vUv;
  layout (location = 0) out vec4 outColor;
  layout (binding = 0) uniform sampler2D renderTarget1;
  layout (binding = 1) uniform sampler2D renderTarget2;
  void main() {
    outColor = mix(
      texture(renderTarget1, vUv),
      texture(renderTarget2, vUv),
      vUv.x
    );
  }
`;

(async function main() {

  const window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU"
  });

  const adapter = await GPU.requestAdapter({ window });

  const device = await adapter.requestDevice();

  const queue = device.getQueue();

  const context = window.getContext("webgpu");

  const swapChainFormat = await context.getSwapChainPreferredFormat(device);

  const swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat
  });

  const renderPipeline = device.createRenderPipeline({
    layout: device.createPipelineLayout({
      bindGroupLayouts: []
    }),
    sampleCount: 1,
    vertexStage: {
      module: device.createShaderModule({ code: vsSrc }),
      entryPoint: "main"
    },
    fragmentStage: {
      module: device.createShaderModule({ code: fsSrc }),
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
      },
      {
        format: swapChainFormat,
        alphaBlend: {},
        colorBlend: {}
      }
    ]
  });

  const blitTexture1 = device.createTexture({
    size: {
      width: window.width,
      height: window.height,
      depth: 1
    },
    arrayLayerCount: 1,
    mipLevelCount: 1,
    sampleCount: 1,
    dimension: "2d",
    format: swapChainFormat,
    usage: GPUTextureUsage.OUTPUT_ATTACHMENT | GPUTextureUsage.SAMPLED
  });
  const blitTextureView1 = blitTexture1.createView({
    format: swapChainFormat
  });

  const blitTexture2 = device.createTexture({
    size: {
      width: window.width,
      height: window.height,
      depth: 1
    },
    arrayLayerCount: 1,
    mipLevelCount: 1,
    sampleCount: 1,
    dimension: "2d",
    format: swapChainFormat,
    usage: GPUTextureUsage.OUTPUT_ATTACHMENT | GPUTextureUsage.SAMPLED
  });
  const blitTextureView2 = blitTexture2.createView({
    format: swapChainFormat
  });

  const blitBindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.FRAGMENT,
        type: "sampled-texture"
      },
      {
        binding: 1,
        visibility: GPUShaderStage.FRAGMENT,
        type: "sampled-texture"
      }
    ]
  });

  const blitBindGroup = device.createBindGroup({
    layout: blitBindGroupLayout,
    // 2 texture inputs
    bindings: [
      {
        binding: 0,
        textureView: blitTextureView1,
        size: 0
      },
      {
        binding: 1,
        textureView: blitTextureView2,
        size: 0
      }
    ]
  });

  const blitPipeline = device.createRenderPipeline({
    layout: device.createPipelineLayout({
      bindGroupLayouts: [blitBindGroupLayout]
    }),
    sampleCount: 1,
    vertexStage: {
      module: device.createShaderModule({ code: blitVsSrc }),
      entryPoint: "main"
    },
    fragmentStage: {
      module: device.createShaderModule({ code: blitFsSrc }),
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
    // 2 color states
    colorStates: [
      {
        format: swapChainFormat,
        alphaBlend: {},
        colorBlend: {}
      }
    ]
  });

  let cachedCommandBuffer = null;
  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

    const backBufferView = swapChain.getCurrentTextureView();

    // since we don't render directly into the swapchain
    // we can now cache command buffers of previous passes
    if (cachedCommandBuffer === null) {
      const commandEncoder = device.createCommandEncoder({});
      const renderPass = commandEncoder.beginRenderPass({
        // 2 color attachments
        colorAttachments: [
          {
            clearColor: { r: 0.125, g: 0.125, b: 0.125, a: 1.0 },
            loadOp: "clear",
            storeOp: "store",
            attachment: blitTextureView1
          },
          {
            clearColor: { r: 0.125, g: 0.125, b: 0.125, a: 1.0 },
            loadOp: "clear",
            storeOp: "store",
            attachment: blitTextureView2
          }
        ]
      });
      renderPass.setPipeline(renderPipeline);
      renderPass.draw(3, 1, 0, 0);
      renderPass.endPass();
      cachedCommandBuffer = commandEncoder.finish();
      console.log("Render Pass Command Buffer is cached");
    }
    // submit cached command buffer
    queue.submit([ cachedCommandBuffer ]);
    // blit to swapchain
    {
      const commandEncoder = device.createCommandEncoder({});
      const blitPass = commandEncoder.beginRenderPass({
        colorAttachments: [{
          clearColor: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
          loadOp: "clear",
          storeOp: "store",
          attachment: backBufferView
        }]
      });
      blitPass.setPipeline(blitPipeline);
      blitPass.setBindGroup(0, blitBindGroup);
      blitPass.draw(3, 1, 0, 0);
      blitPass.endPass();
      const commandBuffer = commandEncoder.finish();
      queue.submit([ commandBuffer ]);
    }
    
    swapChain.present();
    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);

})();
