import WebGPU from "../index.js";
import glMatrix from "gl-matrix";

Object.assign(global, WebGPU);
Object.assign(global, glMatrix);

const vsSrc = `
  #version 450
  #pragma shader_stage(vertex)

  layout(set = 0, binding = 0) uniform Matrices {
    mat4 modelViewProjection;
  } uMatrices;

  layout(location = 0) in vec2 position;
  layout(location = 1) in vec3 color;

  layout(location = 0) out vec4 vColor;

  void main() {
    gl_Position = uMatrices.modelViewProjection * vec4(position, 0.0, 1.0);
    vColor = vec4(color, 1.0);
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
     0.0,  0.5, 1.0, 0.0, 0.0,
    -0.5, -0.5, 0.0, 1.0, 0.0,
     0.5, -0.5, 0.0, 0.0, 1.0
  ]);

  const triangleIndices = new Uint32Array([
    0, 1, 2
  ]);

  const window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU"
  });

  const aspect = Math.abs(window.width / window.height);

  const mModel = mat4.create();
  const mView = mat4.create();
  const mProjection = mat4.create();
  const mModelViewProjection = mat4.create();

  mat4.perspective(mProjection, (2 * Math.PI) / 5, -aspect, 0.1, 4096.0);
  mat4.translate(mView, mView, vec3.fromValues(0, 0, -4));

  const adapter = await GPU.requestAdapter({ window });

  const device = await adapter.requestDevice();

  const queue = device.getQueue();

  const context = window.getContext("webgpu");

  const swapChainFormat = await context.getSwapChainPreferredFormat(device);

  const swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat
  });

  const stagedVertexBuffer = device.createBuffer({
    size: triangleVertices.byteLength,
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
  });
  stagedVertexBuffer.setSubData(0, triangleVertices);

  const stagedIndexBuffer = device.createBuffer({
    size: triangleIndices.byteLength,
    usage: GPUBufferUsage.INDEX | GPUBufferUsage.COPY_DST
  });
  stagedIndexBuffer.setSubData(0, triangleIndices);

  const stagedUniformBuffer = device.createBuffer({
    size: mModelViewProjection.byteLength,
    usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
  });
  stagedUniformBuffer.setSubData(0, mModelViewProjection);

  const uniformBindGroupLayout = device.createBindGroupLayout({
    bindings: [{
      binding: 0,
      visibility: GPUShaderStage.VERTEX,
      type: "uniform-buffer",
      textureDimension: "2D"
    }]
  });

  const layout = device.createPipelineLayout({
    bindGroupLayouts: [ uniformBindGroupLayout ]
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
        arrayStride: 5 * Float32Array.BYTES_PER_ELEMENT,
        stepMode: "vertex",
        attributes: [
          {
            shaderLocation: 0,
            offset: 0 * Float32Array.BYTES_PER_ELEMENT,
            format: "float2"
          },
          {
            shaderLocation: 1,
            offset: 2 * Float32Array.BYTES_PER_ELEMENT,
            format: "float3"
          }
        ]
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

  const uniformBindGroup = device.createBindGroup({
    layout: uniformBindGroupLayout,
    bindings: [{
      binding: 0,
      buffer: stagedUniformBuffer,
      offset: 0,
      size: mModelViewProjection.byteLength
    }]
  });

  let isMouseButtonPressed = false;
  window.onmouseup = e => {
    isMouseButtonPressed = false;
  };
  window.onmousedown = e => {
    isMouseButtonPressed = true;
  };
  window.onmousemove = e => {
    if (!isMouseButtonPressed) return;
    triangleRotateAcceleration += e.movementX * 0.005;
  };

  let triangleRotation = 0;
  let triangleRotateAcceleration = 0;

  let then = Date.now();
  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

    let now = Date.now();
    let delta = (now - then) / 1e3;
    then = now;

    mat4.identity(mModel);
    mat4.rotateY(mModel, mModel, triangleRotation);
    mat4.scale(mModel, mModel, vec3.fromValues(3, 3, 3));
    mat4.multiply(mModelViewProjection, mView, mModel);
    mat4.multiply(mModelViewProjection, mProjection, mModelViewProjection);
    stagedUniformBuffer.setSubData(0, mModelViewProjection);

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
      renderPass.setPipeline(pipeline);
      renderPass.setBindGroup(0, uniformBindGroup);
      renderPass.setVertexBuffer(0, stagedVertexBuffer, 0);
      renderPass.setIndexBuffer(stagedIndexBuffer);
      renderPass.drawIndexed(triangleIndices.length, 1, 0, 0, 0);
      renderPass.endPass();

      const commandBuffer = commandEncoder.finish();
      queue.submit([ commandBuffer ]);
      swapChain.present();

    }

    triangleRotation += triangleRotateAcceleration *= 0.9;
    if (!isMouseButtonPressed) {
      triangleRotation += delta;
    }

    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);

})();
