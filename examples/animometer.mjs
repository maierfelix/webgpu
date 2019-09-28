import WebGPU from "../index.js";
import { performance } from "perf_hooks";

Object.assign(global, WebGPU);

const vsSrc = `
  #version 450
  #pragma shader_stage(vertex)

  layout(std140, set = 0, binding = 0) uniform Time {
    float time;
  };

  layout(std140, set = 1, binding = 0) uniform Uniforms {
    float scale;
    float offsetX;
    float offsetY;
    float scalar;
    float scalarOffset;
  };

  layout(location = 0) in vec2 position;
  layout(location = 1) in vec3 color;

  layout(location = 0) out vec4 vColor;

  void main() {
    float fade = mod(scalarOffset + time * scalar / 10.0, 1.0);
    if (fade < 0.5) {
      fade = fade * 2.0;
    } else {
      fade = (1.0 - fade) * 2.0;
    }
    float xpos = position.x * scale;
    float ypos = position.y * scale;
    float angle = 3.14159 * 2.0 * fade;
    float xrot = xpos * cos(angle) - ypos * sin(angle);
    float yrot = xpos * sin(angle) + ypos * cos(angle);
    xpos = xrot + offsetX;
    ypos = yrot + offsetY;
    vColor = vec4(fade, 1.0 - fade, 0.0, 1.0) + vec4(color, 1);
    gl_Position = vec4(xpos, ypos, 0.0, 1.0);
  }
`;

const fsSrc = `
  #version 450
  #pragma shader_stage(fragment)

  layout(location = 0) in vec4 vColor;

  layout(location = 0) out vec4 outColor;

  void main() {
    outColor = vColor;
  }
`;

(async function main() {

  const triangleVertices = new Float32Array([
     0.0,  0.1, 0.0, 0.0, 0.0,
    -0.1, -0.1, 0.0, 1.0, 0.0,
     0.1, -0.1, 0.0, 0.0, 1.0
  ]);

  const window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU",
    resizable: false
  });

  const adapter = await GPU.requestAdapter({ window });

  const device = await adapter.requestDevice();

  const context = window.getContext("webgpu");

  const swapChainFormat = await context.getSwapChainPreferredFormat(device);

  const swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat,
    usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.OUTPUT_ATTACHMENT
  });

  const stagedVertexBuffer = device.createBuffer({
    size: BigInt(triangleVertices.byteLength),
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
  });
  stagedVertexBuffer.setSubData(0n, triangleVertices);

  const vertexShaderModule = device.createShaderModule({ code: vsSrc });
  const fragmentShaderModule = device.createShaderModule({ code: fsSrc });

  const timeBindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.VERTEX,
        type: "uniform-buffer"
      },
    ]
  });

  const bindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.VERTEX,
        type: "uniform-buffer"
      },
    ]
  });

  const dynamicBindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.VERTEX,
        type: "uniform-buffer",
        dynamic: true
      },
    ]
  });

  const pipelineLayout = device.createPipelineLayout({
    bindGroupLayouts: [timeBindGroupLayout, bindGroupLayout]
  });

  const dynamicPipelineLayout = device.createPipelineLayout({
    bindGroupLayouts: [timeBindGroupLayout, dynamicBindGroupLayout]
  });

  const pipelineDesc = {
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
    vertexInput: {
      indexFormat: "uint32",
      buffers: [
        {
          stride: BigInt(5 * Float32Array.BYTES_PER_ELEMENT),
          stepMode: "vertex",
          attributes: [
            {
              shaderLocation: 0,
              offset: BigInt(0 * Float32Array.BYTES_PER_ELEMENT),
              format: "float2"
            },
            {
              shaderLocation: 1,
              offset: BigInt(2 * Float32Array.BYTES_PER_ELEMENT),
              format: "float3"
            }
          ]
        },
      ]
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
  };

  const pipeline = device.createRenderPipeline(Object.assign({
    layout: pipelineLayout
  }, pipelineDesc));

  const dynamicPipeline = device.createRenderPipeline(Object.assign({
    layout: dynamicPipelineLayout
  }, pipelineDesc));

  function configure() {
    const numTriangles = 25000;
    const uniformBytes = 5 * Float32Array.BYTES_PER_ELEMENT;
    const alignedUniformBytes = Math.ceil(uniformBytes / 256) * 256;
    const alignedUniformFloats = alignedUniformBytes / Float32Array.BYTES_PER_ELEMENT;
    const uniformBuffer = device.createBuffer({
      size: BigInt(numTriangles * alignedUniformBytes + Float32Array.BYTES_PER_ELEMENT),
      usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.UNIFORM
    });
    const uniformBufferData = new Float32Array(numTriangles * alignedUniformFloats);
    const bindGroups = new Array(numTriangles);
    for (let i = 0; i < numTriangles; ++i) {
      uniformBufferData[alignedUniformFloats * i + 0] = Math.random() * 0.2 + 0.2;        // scale
      uniformBufferData[alignedUniformFloats * i + 1] = 0.9 * 2 * (Math.random() - 0.5);  // offsetX
      uniformBufferData[alignedUniformFloats * i + 2] = 0.9 * 2 * (Math.random() - 0.5);  // offsetY
      uniformBufferData[alignedUniformFloats * i + 3] = Math.random() * 1.5 + 0.5;       // scalar
      uniformBufferData[alignedUniformFloats * i + 4] = Math.random() * 10;               // scalarOffset
      bindGroups[i] = device.createBindGroup({
        layout: bindGroupLayout,
        bindings: [{
          binding: 0,
          buffer: uniformBuffer,
          offset: BigInt(i * alignedUniformBytes),
          size: BigInt(6 * Float32Array.BYTES_PER_ELEMENT)
        }]
      });
    }
    const dynamicBindGroup = device.createBindGroup({
      layout: dynamicBindGroupLayout,
      bindings: [{
        binding: 0,
        buffer: uniformBuffer,
        offset: BigInt(0),
        size: BigInt(6 * Float32Array.BYTES_PER_ELEMENT)
      }],
    });
    const timeOffset = numTriangles * alignedUniformBytes;
    const timeBindGroup = device.createBindGroup({
      layout: timeBindGroupLayout,
      bindings: [{
        binding: 0,
        buffer: uniformBuffer,
        offset: BigInt(timeOffset),
        size: BigInt(Float32Array.BYTES_PER_ELEMENT)
      }]
    });
    // Chrome currently crashes with |setSubData| too large.
    const maxSetSubDataLength = 14 * 1024 * 1024 / Float32Array.BYTES_PER_ELEMENT;
    for (let offset = 0; offset < uniformBufferData.length; offset += maxSetSubDataLength) {
      uniformBuffer.setSubData(
        BigInt(offset),
        new Float32Array(
          uniformBufferData.buffer,
          offset * Float32Array.BYTES_PER_ELEMENT,
          Math.min(uniformBufferData.length - offset, maxSetSubDataLength)
        )
      );
    }
    function createCommandBuffer(textureView) {
      const commandEncoder = device.createCommandEncoder({});
      const renderPassDescriptor = {
        colorAttachments: [{
          clearColor: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
          loadOp: "clear",
          storeOp: "store",
          attachment: textureView
        }],
      };
      const passEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
      if (/*settings.dynamicOffsets*/true) {
        passEncoder.setPipeline(dynamicPipeline);
      } else {
        passEncoder.setPipeline(pipeline);
      }
      passEncoder.setVertexBuffers(0, [stagedVertexBuffer], [0n]);
      passEncoder.setBindGroup(0, timeBindGroup);
      const dynamicOffsets = [0n];
      for (let i = 0; i < numTriangles; ++i) {
        if (/*settings.dynamicOffsets*/true) {
          dynamicOffsets[0] = BigInt(i * alignedUniformBytes);
          passEncoder.setBindGroup(1, dynamicBindGroup, dynamicOffsets);
        } else {
          passEncoder.setBindGroup(1, bindGroups[i]);
        }
        passEncoder.draw(3, 1, 0, 0);
      }
      passEncoder.endPass();
      return commandEncoder.finish();
    }
    const reusableFrames = [];
    for (let i = 0; i < 2; ++i) {
      const texture = device.createTexture({
        size: { width: window.width, height: window.height, depth: 1 },
        arrayLayerCount: 1,
        mipLevelCount: 1,
        sampleCount: 1,
        dimension: "2d",
        format: swapChainFormat,
        usage: GPUTextureUsage.OUTPUT_ATTACHMENT | GPUTextureUsage.COPY_SRC,
      });
      reusableFrames.push({
        commandBuffer: createCommandBuffer(texture.createView({ format: swapChainFormat })),
        texture,
      });
    }
    const uniformTime = new Float32Array([0]);
    let i = 0;
    let startTime = undefined;
    return function doDraw(timestamp) {
      if (startTime === undefined) {
        startTime = timestamp;
      }
      uniformTime[0] = (timestamp - startTime) / 1000;
      uniformBuffer.setSubData(BigInt(timeOffset), uniformTime);
      if (/*settings.reuseCommandBuffers*/true) {
        const { commandBuffer, texture } = reusableFrames[(i + 1) % reusableFrames.length];
        i++;
        const commandEncoder = device.createCommandEncoder({});
        commandEncoder.copyTextureToTexture(
          {
            texture: texture,
            mipLevel: 0,
            arrayLayer: 0,
            origin: { x: 0, y: 0, z: 0 }
          },
          {
            texture: swapChain.getCurrentTexture(),
            mipLevel: 0,
            arrayLayer: 0,
            origin: { x: 0, y: 0, z: 0 }
          },
          {
            width: window.width,
            height: window.height,
            depth: 1,
          },
        );
        device.getQueue().submit([
          commandBuffer,
          commandEncoder.finish(),
        ]);
      } else {
        const textureView = swapChain.getCurrentTexture().createView({ format: swapChainFormat });
        device.getQueue().submit([ createCommandBuffer(textureView) ]);
      }
    }
  }

  let doDraw = configure();
  const updateSettings = () => { doDraw = configure(); }

  let previousFrameTimestamp = undefined;
  let jsTimeAvg = undefined;
  let frameTimeAvg = undefined;
  let updateDisplay = true;
  let i = 0;
  let then = performance.now();
  let timestamp = 0;
  function onFrame() {
    let now = performance.now();
    timestamp += (now - then);
    then = now;
    let frameTime = 0;
    if (previousFrameTimestamp !== undefined) {
      frameTime = timestamp - previousFrameTimestamp;
    }
    previousFrameTimestamp = timestamp;
    const start = performance.now();
    doDraw(timestamp);
    const jsTime = performance.now() - start;
    if (frameTimeAvg === undefined) {
      frameTimeAvg = frameTime;
    }
    if (jsTimeAvg === undefined) {
      jsTimeAvg = jsTime;
    }
    const w = 0.2;
    frameTimeAvg = (1 - w) * frameTimeAvg + w * frameTime;
    jsTimeAvg = (1 - w) * jsTimeAvg + w * jsTime;
    if (updateDisplay) {
      window.title = `Avg Javascript: ${jsTimeAvg.toFixed(2)} ms\nAvg Frame: ${frameTimeAvg.toFixed(2)} ms`;

    }
    swapChain.present(swapChain.getCurrentTexture());
    window.pollEvents();
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);
  }
  setTimeout(onFrame, 1e3 / 60);

})();
