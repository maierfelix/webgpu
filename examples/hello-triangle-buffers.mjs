import WebGPU from "../index.js";

Object.assign(global, WebGPU);

const vsSrc = `
  #version 450
  #pragma shader_stage(vertex)
  layout(location = 0) in vec2 position;
  void main() {
    gl_Position = vec4(position, 0.0, 1.0);
  }
`;

const fsSrc = `
  #version 450
  #pragma shader_stage(fragment)
  layout(location = 0) out vec4 outColor;
  void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
  }
`;

(async function main() {

  const triangleVertices = new Float32Array([
     0.0,  0.5,
    -0.5, -0.5,
     0.5, -0.5
  ]);

  const triangleIndices = new Uint32Array([
    0, 1, 2
  ]);

  const window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU"
  });

  const context = window.getContext("webgpu");

  const adapter = await GPU.requestAdapter({ window });

  const device = await adapter.requestDevice();

  const queue = device.getQueue();

  const swapChainFormat = await context.getSwapChainPreferredFormat(device);

  const swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat
  });

  // demonstrate verbose staging process
  const stagingVertexBuffer = device.createBuffer({
    size: triangleVertices.byteLength,
    usage: GPUBufferUsage.MAP_WRITE | GPUBufferUsage.COPY_SRC
  });
  const stagingVertexBufferView = await stagingVertexBuffer.mapWriteAsync();
  stagingVertexBuffer.unmap();

  const stagedVertexBuffer = device.createBuffer({
    size: triangleVertices.byteLength,
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
  });

  // staging->staged buffer
  const bufferCopyEncoder = device.createCommandEncoder({});
  bufferCopyEncoder.copyBufferToBuffer(
    stagingVertexBuffer,
    0,
    stagedVertexBuffer,
    0,
    triangleVertices.byteLength
  );
  queue.submit([ bufferCopyEncoder.finish() ]);

  // staging shortcut using buffer.setSubData
  const stagedIndexBuffer = device.createBuffer({
    size: triangleIndices.byteLength,
    usage: GPUBufferUsage.INDEX | GPUBufferUsage.COPY_DST
  });
  stagedIndexBuffer.setSubData(0, triangleIndices);

  const layout = device.createPipelineLayout({
    bindGroupLayouts: []
  });

  const vertexShaderModule = device.createShaderModule({ code: vsSrc });
  const fragmentShaderModule = device.createShaderModule({ code: fsSrc });

  const pipeline = device.createRenderPipeline({
    layout,
    sampleCount: 1,
    vertexStage: {
      module: vertexShaderModule,
      entryPoint: "main"
    },
    fragmentStage: {
      module: fragmentShaderModule,
      entryPoint: "main"
    },
    primitiveTopology: "triangle-list",
    vertexState: {
      indexFormat: "uint32",
      vertexBuffers: [{
          arrayStride: 2 * Float32Array.BYTES_PER_ELEMENT,
          stepMode: "vertex",
          attributes: [{
            shaderLocation: 0,
            offset: 0,
            format: "float2"
          }]
      }]
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

  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

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
    renderPass.setPipeline(pipeline);
    renderPass.setVertexBuffer(0, stagedVertexBuffer, 0);
    renderPass.setIndexBuffer(stagedIndexBuffer);
    renderPass.drawIndexed(triangleIndices.length, 1, 0, 0, 0);
    renderPass.endPass();

    const commandBuffer = commandEncoder.finish();
    queue.submit([ commandBuffer ]);
    swapChain.present();
    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);

})();
