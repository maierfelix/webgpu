import WebGPU from "../index.js";

Object.assign(global, WebGPU);

const rayGenSrc = `
  #version 460
  #extension GL_NV_ray_tracing : require
  #pragma shader_stage(raygen)

  void main() {
    
  }
`;

(async function main() {

  const modelVertices = new Float32Array([
    // Front face
    -1.0, -1.0,  1.0,
     1.0, -1.0,  1.0,
     1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    // Back face
    -1.0, -1.0, -1.0,
    -1.0,  1.0, -1.0,
     1.0,  1.0, -1.0,
     1.0, -1.0, -1.0,
    // Top face
    -1.0,  1.0, -1.0,
    -1.0,  1.0,  1.0,
     1.0,  1.0,  1.0,
     1.0,  1.0, -1.0,
    // Bottom face
    -1.0, -1.0, -1.0,
     1.0, -1.0, -1.0,
     1.0, -1.0,  1.0,
    -1.0, -1.0,  1.0,
    // Right face
     1.0, -1.0, -1.0,
     1.0,  1.0, -1.0,
     1.0,  1.0,  1.0,
     1.0, -1.0,  1.0,
    // Left face
    -1.0, -1.0, -1.0,
    -1.0, -1.0,  1.0,
    -1.0,  1.0,  1.0,
    -1.0,  1.0, -1.0
  ]);

  const modelIndices = new Uint32Array([
    0,  1,  2,
    2,  3,  0,
    4,  5,  6,
    6,  7,  4,
    8,  9,  10,
    10, 11, 8,
    12, 13, 14,
    14, 15, 12,
    16, 17, 18,
    18, 19, 16,
    20, 21, 22,
    22, 23, 20
  ]);

  const window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU"
  });

  const adapter = await GPU.requestAdapter({
    window,
    preferredBackend: "Vulkan"
  });

  const device = await adapter.requestDevice();

  const queue = device.getQueue();

  const context = window.getContext("webgpu");

  const swapChainFormat = await context.getSwapChainPreferredFormat(device);

  const swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat
  });

  const rayGenShaderModule = device.createShaderModule({
    code: rayGenSrc
  });
  console.log(rayGenShaderModule);

  console.log(GPUShaderStage);
  console.log(GPURayTracingAccelerationContainerFlag);

  const stagedVertexBuffer = device.createBuffer({
    size: modelVertices.byteLength,
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
  });
  stagedVertexBuffer.setSubData(0, modelVertices);

  const stagedIndexBuffer = device.createBuffer({
    size: modelIndices.byteLength,
    usage: GPUBufferUsage.INDEX | GPUBufferUsage.COPY_DST
  });
  stagedIndexBuffer.setSubData(0, modelIndices);

  const geometry0 = {
    type: "triangles",
    vertexBuffer: stagedVertexBuffer,
    vertexFormat: "float32",
    vertexStride: 3 * Float32Array.BYTES_PER_ELEMENT,
    indexBuffer: stagedIndexBuffer,
    indexFormat: "uint32",
  };

  const geometryContainer0 = device.createRayTracingAccelerationContainer({
    level: "bottom",
    flag: GPURayTracingAccelerationContainerFlag.PREFER_FAST_TRACE,
    geometries: [ geometry0 ]
  });
  console.log(geometryContainer0);

  const instance0 = {
    flags: GPURayTracingAccelerationInstanceFlag.TRIANGLE_CULL_DISABLE,
    mask: 0xFF,
    instanceId: 0,
    instanceOffset: 0x0,
    transform: new Float32Array(12),
    geometryContainer: geometryContainer0
  };

  const instanceContainer0 = device.createRayTracingAccelerationContainer({
    level: "top",
    instances: [ instance0 ]
  });
  console.log(instanceContainer0);

  const commandEncoder = device.createCommandEncoder({});
  console.log(0);
  commandEncoder.buildRayTracingAccelerationContainer(geometryContainer0);
  commandEncoder.buildRayTracingAccelerationContainer(instanceContainer0);
  const commandBuffer = commandEncoder.finish();
  console.log(1);
  queue.submit([ commandBuffer ]);
  console.log(2);

  const shaderBindingTable = device.createRayTracingShaderBindingTable({
    hitShaders: [],
    anyHitShaders: [],
    closestHitShaders: [],
    missShaders: [],
    intersectionShaders: [],
    generationShaders: []
  });
  console.log(shaderBindingTable);

/*
  const bindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.RAY_GENERATION | GPUShaderStage.RAY_CLOSEST_HIT,
        type: "acceleration-structure"
      },
      {
        binding: 1,
        visibility: GPUShaderStage.RAY_GENERATION | GPUShaderStage.RAY_CLOSEST_HIT,
        type: "storage-texture"
      }
    ]
  });

  const layout = device.createPipelineLayout({
    bindGroupLayouts: [bindGroupLayout]
  });

  const rtPipeline = device.createRayTracingPipeline({
    layout,
    shaderBindingTable,
    accelerationStructures: [accelerationStructure],
  });

  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

    const backBuffer = swapChain.getCurrentTexture();
    const backBufferView = backBuffer.createView({
      format: swapChainFormat
    });

    {
      const commandEncoder = device.createCommandEncoder({});
      const rayTracingPass = commandEncoder.beginRenderPass({});
      rayTracingPass.setPipeline(rtPipeline);
      rayTracingPass.traceRays(window.width, window.height);
      rayTracingPass.endPass();
    }

    const commandBuffer = commandEncoder.finish();
    queue.submit([ commandBuffer ]);
    swapChain.present(backBuffer);
    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);
*/

})();
