import WebGPU from "../index.js";

const vertexShaderGLSL = `
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

const fragmentShaderGLSL = `
  #version 450
  #pragma shader_stage(fragment)
  layout(location = 0) out vec4 outColor;
  void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
  }
`;

(async function main() {

  const adapter = await WebGPU.GPU.requestAdapter();
  console.log("Adapter:", adapter);
  console.log("Adapter Name:", adapter.name);
  console.log("Adapter Extensions:", adapter.extensions);

  const device = await adapter.requestDevice();
  console.log("Device:", device);

  const swapChainFormat = "BGRA8 unorm";

  const context = WebGPU.GPU.getContext("webgpu");
  console.log("Canvas Context:", context);

  const swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat
  });
  console.log("Swapchain:", swapChain);

  const layout = device.createPipelineLayout({
    bindGroupLayouts: []
  });
  console.log("Layout:", layout);

  const vertexShaderModule = device.createShaderModule({
    code: vertexShaderGLSL
  });
  console.log("Vertex Shader Module:", vertexShaderModule);

  const fragmentShaderModule = device.createShaderModule({
    code: fragmentShaderGLSL
  });
  console.log("Fragment Shader Module:", fragmentShaderModule);

  const pipeline = device.createRenderPipeline({
    layout,
    vertexStage: {
      module: vertexShaderModule,
      entryPoint: "main"
    },
    fragmentStage: {
      module: fragmentShaderModule,
      entryPoint: "main"
    },
    primitiveTopology: "triangle list",
    vertexInput: {
      indexFormat: "uint32",
      buffers: []
    },
    rasterizationState: {
      frontFace: "CCW",
      cullMode: "none"
    },
    colorStates: [{
      format: swapChainFormat,
      alphaBlend: {},
      colorBlend: {}
    }]
  });
  console.log("Pipeline:", pipeline);

  const queue = device.getQueue();
  console.log("Queue:", queue);

  function onFrame() {
    const commandEncoder = device.createCommandEncoder({});
    const textureView = swapChain.getCurrentTexture().createView();
    const passEncoder = commandEncoder.beginRenderPass({
      colorAttachments: [{
        clearColor: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
        loadOp: "load",
        storeOp: "store",
        attachment: textureView
      }]
    });
    passEncoder.setPipeline(pipeline);
    passEncoder.draw(3, 1, 0, 0);
    passEncoder.endPass();
    queue.submit([
      commandEncoder.finish()
    ]);
  };
  setTimeout(onFrame, 1e3 / 60);

})();
