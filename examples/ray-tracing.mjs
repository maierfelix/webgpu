import WebGPU from "../index.js";

Object.assign(global, WebGPU);

const rayGenSrc = `
  #version 460
  #extension GL_NV_ray_tracing : require
  #pragma shader_stage(raygen)

  layout(binding = 0, set = 0) uniform accelerationStructureNV TLAS;

  layout(location = 0) rayPayloadNV vec3 hitValue;

  mat4 mViewInverse = mat4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
  );

  mat4 mProjectionInverse = mat4(
    -8.540441513061523, -0, -0, -0,
    -0, -6.405331134796143, -0, -0,
    -0, -0, -12.497565269470215, -4.999025821685791,
    0, -0, 11.502447128295898, 5.000978946685791
  );

  void main() {
    const vec2 pixelCenter = vec2(gl_LaunchIDNV.xy) + vec2(0.5);
    const vec2 uv = pixelCenter / vec2(gl_LaunchSizeNV.xy);
    vec2 d = uv * 2.0 - 1.0;

    vec4 origin = mViewInverse * vec4(0,0,0,1);
    vec4 target = mProjectionInverse * vec4(d.x, d.y, 1, 1);
    vec4 direction = mViewInverse * vec4(normalize(target.xyz), 0);

    //traceNV(TLAS, gl_RayFlagsOpaqueNV, 0xff, 0, 0, 0, origin.xyz, 0.1, direction.xyz, 1024.0, 0);

    //imageStore(image, ivec2(gl_LaunchIDNV.xy), vec4(hitValue, 0.0));
  }
`;

const rayCHitSrc = `
  #version 460
  #extension GL_NV_ray_tracing : require
  #extension GL_EXT_nonuniform_qualifier : enable
  #pragma shader_stage(closest)

  layout(location = 0) rayPayloadInNV vec3 hitValue;
  hitAttributeNV vec3 attribs;

  void main() {
    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    hitValue = barycentricCoords;
  }
`;

const rayMissSrc = `
  #version 460
  #extension GL_NV_ray_tracing : require
  #pragma shader_stage(miss)

  layout(location = 0) rayPayloadInNV vec3 hitValue;

  void main() {
    hitValue = vec3(0.0, 0.0, 0.2);
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

  const rayCHitShaderModule = device.createShaderModule({
    code: rayCHitSrc
  });

  const rayMissShaderModule = device.createShaderModule({
    code: rayMissSrc
  });

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

  const commandEncoder = device.createCommandEncoder({});
  commandEncoder.buildRayTracingAccelerationContainer(geometryContainer0);
  commandEncoder.buildRayTracingAccelerationContainer(instanceContainer0);
  const commandBuffer = commandEncoder.finish();
  queue.submit([ commandBuffer ]);

  const shaderBindingTable = device.createRayTracingShaderBindingTable({
    shaders: [
      {
        module: rayGenShaderModule,
        stage: GPUShaderStage.RAY_GENERATION
      },
      {
        module: rayCHitShaderModule,
        stage: GPUShaderStage.RAY_CLOSEST_HIT
      },
      {
        module: rayMissShaderModule,
        stage: GPUShaderStage.RAY_MISS
      }
    ]
  });

  const bindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.RAY_GENERATION | GPUShaderStage.RAY_CLOSEST_HIT | GPUShaderStage.RAY_MISS,
        type: "acceleration-container",
        textureDimension: "2D"
      }
    ]
  });

  const bindGroup = device.createBindGroup({
    layout: bindGroupLayout,
    bindings: [{
      binding: 0,
      accelerationContainer: instanceContainer0,
      size: 0
    }]
  });

  const layout = device.createPipelineLayout({
    bindGroupLayouts: [bindGroupLayout]
  });

  const pipeline = device.createRayTracingPipeline({
    layout,
    rayTracingState: {
      shaderBindingTable,
      maxRecursionDepth: 1
    }
  });

  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

    const backBufferView = swapChain.getCurrentTextureView();

    const commandEncoder = device.createCommandEncoder({});
    {
      const rayTracingPass = commandEncoder.beginRayTracingPass({});
      rayTracingPass.setPipeline(pipeline);
      rayTracingPass.setBindGroup(0, bindGroup);
      rayTracingPass.traceRays(window.width, window.height, 1);
      rayTracingPass.endPass();
    }
    let commandBuffer = commandEncoder.finish();
    queue.submit([ commandBuffer ]);

    swapChain.present();
    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);

})();
