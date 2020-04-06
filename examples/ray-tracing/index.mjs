import WebGPU from "../../index.js";

import fs from "fs";
import glMatrix from "gl-matrix";

Object.assign(global, WebGPU);
Object.assign(global, glMatrix);

(async function main() {

  let window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU",
    resizable: false
  });

  let adapter = await GPU.requestAdapter({
    window,
    preferredBackend: "Vulkan"
  });

  let device = await adapter.requestDevice();

  let queue = device.getQueue();

  let context = window.getContext("webgpu");

  let swapChainFormat = await context.getSwapChainPreferredFormat(device);

  let swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat
  });

  let aspect = Math.abs(window.width / window.height);

  let mView = mat4.create();
  let mProjection = mat4.create();

  mat4.perspective(mProjection, (2 * Math.PI) / 5, -aspect, 0.1, 4096.0);

  mat4.translate(mView, mView, vec3.fromValues(0, 0, -2));

  // invert
  mat4.invert(mView, mView);
  mat4.invert(mProjection, mProjection);
  mProjection[5] *= -1.0;

  let baseShaderPath = `examples/ray-tracing/shaders`;

  // rasterization shaders
  let vertexShaderModule = device.createShaderModule({
    code: fs.readFileSync(`${baseShaderPath}/screen.vert`, "utf-8")
  });
  let fragmentShaderModule = device.createShaderModule({
    code: fs.readFileSync(`${baseShaderPath}/screen.frag`, "utf-8")
  });

  // ray-tracing shaders
  let rayGenShaderModule = device.createShaderModule({
    code: fs.readFileSync(`${baseShaderPath}/ray-generation.rgen`, "utf-8")
  });
  let rayCHitShaderModule = device.createShaderModule({
    code: fs.readFileSync(`${baseShaderPath}/ray-closest-hit.rchit`, "utf-8")
  });
  let rayMissShaderModule = device.createShaderModule({
    code: fs.readFileSync(`${baseShaderPath}/ray-miss.rmiss`, "utf-8")
  });

  // this storage buffer is used as a pixel buffer
  // the result of the ray tracing pass gets written into it
  // and it gets copied to the screen in the rasterization pass
  let pixelBufferSize = window.width * window.height * 4 * Float32Array.BYTES_PER_ELEMENT;
  let pixelBuffer = device.createBuffer({
    size: pixelBufferSize,
    usage: GPUBufferUsage.STORAGE
  });

  let triangleVertices = new Float32Array([
     1.0,  1.0, 0.0,
    -1.0,  1.0, 0.0,
     0.0, -1.0, 0.0
  ]);
  let triangleVertexBuffer = device.createBuffer({
    size: triangleVertices.byteLength,
    usage: GPUBufferUsage.COPY_DST
  });
  triangleVertexBuffer.setSubData(0, triangleVertices);

  let triangleIndices = new Uint32Array([
    0, 1, 2
  ]);
  let triangleIndexBuffer = device.createBuffer({
    size: triangleIndices.byteLength,
    usage: GPUBufferUsage.COPY_DST
  });
  triangleIndexBuffer.setSubData(0, triangleIndices);

  // create a geometry container
  // which holds references to our geometry buffers
  let geometryContainer = device.createRayTracingAccelerationContainer({
    level: "bottom",
    flags: GPURayTracingAccelerationContainerFlag.PREFER_FAST_TRACE,
    geometries: [
      {
        flags: GPURayTracingAccelerationGeometryFlag.OPAQUE,
        type: "triangles",
        vertex: {
          buffer: triangleVertexBuffer,
          format: "float3",
          stride: 3 * Float32Array.BYTES_PER_ELEMENT,
          count: triangleVertices.length
        },
        index: {
          buffer: triangleIndexBuffer,
          format: "uint32",
          count: triangleIndices.length
        }
      }
    ]
  });

  // create an instance container
  // which contains object instances with transforms
  // and links to a geometry container to be used
  let instanceContainer = device.createRayTracingAccelerationContainer({
    level: "top",
    flags: GPURayTracingAccelerationContainerFlag.PREFER_FAST_TRACE,
    instances: [
      {
        flags: GPURayTracingAccelerationInstanceFlag.TRIANGLE_CULL_DISABLE,
        mask: 0xFF,
        instanceId: 0,
        instanceOffset: 0x0,
        transform: {
          translation: { x: 0, y: 0, z: 0 },
          rotation: { x: 0, y: 0, z: 0 },
          scale: { x: 1, y: 1, z: 1 }
        },
        geometryContainer: geometryContainer
      }
    ]
  });

  // build the containers (the order is important)
  // geometry containers have to be built before
  // an instance container, if it has a geometry reference into it
  {
    let commandEncoder = device.createCommandEncoder({});
    commandEncoder.buildRayTracingAccelerationContainer(geometryContainer);
    commandEncoder.buildRayTracingAccelerationContainer(instanceContainer);
    queue.submit([ commandEncoder.finish() ]);
  }

  // a collection of shader modules which get dynamically
  // invoked, for example when calling traceNV
  let shaderBindingTable = device.createRayTracingShaderBindingTable({
    // stages are a collection of shaders
    // which get indexed in groups
    stages: [
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
    ],
    // groups can index the shaders in stages
    // generalIndex: ray generation or ray miss stage index
    // anyHitIndex: ray any-hit stage index
    // closestHitIndex: ray closest-hit stage index
    // intersectionIndex: ray intersection stage index
    groups: [
      // generation group
      {
        type: "general",
        generalIndex: 0, // ray generation shader index
        anyHitIndex: -1,
        closestHitIndex: -1,
        intersectionIndex: -1
      },
      // hit group
      {
        type: "triangles-hit-group",
        generalIndex: -1,
        anyHitIndex: -1,
        closestHitIndex: 1, // ray closest-hit shader index
        intersectionIndex: -1
      },
      // miss group
      {
        type: "general",
        generalIndex: 2, // ray miss shader index
        anyHitIndex: -1,
        closestHitIndex: -1,
        intersectionIndex: -1
      }
    ]
  });

  let rtBindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.RAY_GENERATION,
        type: "acceleration-container"
      },
      {
        binding: 1,
        visibility: GPUShaderStage.RAY_GENERATION,
        type: "storage-buffer"
      },
      {
        binding: 2,
        visibility: GPUShaderStage.RAY_GENERATION,
        type: "uniform-buffer"
      }
    ]
  });

  let cameraData = new Float32Array(
    // (mat4) view
    mView.byteLength +
    // (mat4) projection
    mProjection.byteLength
  );
  let cameraUniformBuffer = device.createBuffer({
    size: cameraData.byteLength,
    usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM
  });
  // fill in the data
  {
    let offset = 0x0;
    cameraData.set(mView, offset);
    offset += mView.length;
    cameraData.set(mProjection, offset);
    offset += mProjection.length;
  }
  cameraUniformBuffer.setSubData(0, cameraData);

  let rtBindGroup = device.createBindGroup({
    layout: rtBindGroupLayout,
    bindings: [
      {
        binding: 0,
        accelerationContainer: instanceContainer,
        offset: 0,
        size: 0
      },
      {
        binding: 1,
        buffer: pixelBuffer,
        offset: 0,
        size: pixelBufferSize
      },
      {
        binding: 2,
        buffer: cameraUniformBuffer,
        offset: 0,
        size: cameraData.byteLength
      }
    ]
  });

  let rtPipeline = device.createRayTracingPipeline({
    layout: device.createPipelineLayout({
      bindGroupLayouts: [rtBindGroupLayout]
    }),
    rayTracingState: {
      shaderBindingTable,
      maxRecursionDepth: 1
    }
  });

  let resolutionData = new Float32Array([
    window.width, window.height
  ]);
  let resolutionUniformBuffer = device.createBuffer({
    size: resolutionData.byteLength,
    usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM
  });
  resolutionUniformBuffer.setSubData(0, resolutionData);

  let renderBindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.FRAGMENT,
        type: "storage-buffer"
      },
      {
        binding: 1,
        visibility: GPUShaderStage.FRAGMENT,
        type: "uniform-buffer"
      }
    ]
  });

  let renderBindGroup = device.createBindGroup({
    layout: renderBindGroupLayout,
    bindings: [
      {
        binding: 0,
        buffer: pixelBuffer,
        offset: 0,
        size: pixelBufferSize
      },
      {
        binding: 1,
        buffer: resolutionUniformBuffer,
        offset: 0,
        size: resolutionData.byteLength
      }
    ]
  });

  let renderPipeline = device.createRenderPipeline({
    layout: device.createPipelineLayout({
      bindGroupLayouts: [renderBindGroupLayout]
    }),
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
      vertexBuffers: []
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

    let backBufferView = swapChain.getCurrentTextureView();

    // ray tracing pass
    {
      let commandEncoder = device.createCommandEncoder({});
      let passEncoder = commandEncoder.beginRayTracingPass({});
      passEncoder.setPipeline(rtPipeline);
      passEncoder.setBindGroup(0, rtBindGroup);
      passEncoder.traceRays(
        0, // sbt ray-generation offset
        1, // sbt ray-hit offset
        2, // sbt ray-miss offset
        window.width,  // query width dimension
        window.height, // query height dimension
        1              // query depth dimension
      );
      passEncoder.endPass();
      queue.submit([ commandEncoder.finish() ]);

    }
    // rasterization pass
    // the rasterization's pass only use right now,
    // is to bring the pixel buffer we write into from the
    // ray tracing pass, to the screen
    {
      let commandEncoder = device.createCommandEncoder({});
      let passEncoder = commandEncoder.beginRenderPass({
        colorAttachments: [{
          clearColor: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
          loadOp: "clear",
          storeOp: "store",
          attachment: backBufferView
        }]
      });
      passEncoder.setPipeline(renderPipeline);
      passEncoder.setBindGroup(0, renderBindGroup);
      passEncoder.draw(3, 1, 0, 0);
      passEncoder.endPass();
      queue.submit([ commandEncoder.finish() ]);
    }

    swapChain.present();
    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);

})();
