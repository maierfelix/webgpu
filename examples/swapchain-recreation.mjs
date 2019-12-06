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

  layout(location = 0) in vec3 position;
  layout(location = 1) in vec3 color;

  layout(location = 0) out vec4 vColor;

  void main() {
    gl_Position = uMatrices.modelViewProjection * vec4(position, 1.0);
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

  const modelVertices = new Float32Array([
    // Front face
    -1.0, -1.0,  1.0, 1.0, 0.0, 0.0,
     1.0, -1.0,  1.0, 0.0, 1.0, 0.0,
     1.0,  1.0,  1.0, 0.0, 0.0, 1.0,
    -1.0,  1.0,  1.0, 0.0, 1.0, 0.0,
    // Back face
    -1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
    -1.0,  1.0, -1.0, 0.0, 1.0, 0.0,
     1.0,  1.0, -1.0, 0.0, 0.0, 1.0,
     1.0, -1.0, -1.0, 0.0, 1.0, 0.0,
    // Top face
    -1.0,  1.0, -1.0, 1.0, 0.0, 0.0,
    -1.0,  1.0,  1.0, 0.0, 1.0, 0.0,
     1.0,  1.0,  1.0, 0.0, 0.0, 1.0,
     1.0,  1.0, -1.0, 0.0, 1.0, 0.0,
    // Bottom face
    -1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
     1.0, -1.0, -1.0, 0.0, 1.0, 0.0,
     1.0, -1.0,  1.0, 0.0, 0.0, 1.0,
    -1.0, -1.0,  1.0, 0.0, 1.0, 0.0,
    // Right face
     1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
     1.0,  1.0, -1.0, 0.0, 1.0, 0.0,
     1.0,  1.0,  1.0, 0.0, 0.0, 1.0,
     1.0, -1.0,  1.0, 0.0, 1.0, 0.0,
    // Left face
    -1.0, -1.0, -1.0, 1.0, 0.0, 0.0,
    -1.0, -1.0,  1.0, 0.0, 1.0, 0.0,
    -1.0,  1.0,  1.0, 0.0, 0.0, 1.0,
    -1.0,  1.0, -1.0, 0.0, 1.0, 0.0
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

  const mModel = mat4.create();
  const mView = mat4.create();
  const mProjection = mat4.create();
  const mModelViewProjection = mat4.create();

  mat4.perspective(mProjection, (2 * Math.PI) / 5, -(window.width / window.height), 0.1, 4096.0);
  mat4.rotateX(mView, mView, -35 * Math.PI / 180);
  mat4.translate(mView, mView, vec3.fromValues(0, 6, -8));

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
    size: modelVertices.byteLength,
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
  });
  stagedVertexBuffer.setSubData(0, modelVertices);

  const stagedIndexBuffer = device.createBuffer({
    size: modelIndices.byteLength,
    usage: GPUBufferUsage.INDEX | GPUBufferUsage.COPY_DST
  });
  stagedIndexBuffer.setSubData(0, modelIndices);

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
    depthStencilState: {
      depthWriteEnabled: true,
      depthCompare: "less",
      format: "depth24plus-stencil8",
      stencilFront: {},
      stencilBack: {},
    },
    vertexState: {
      indexFormat: "uint32",
      vertexBuffers: [
        {
          arrayStride: 6 * Float32Array.BYTES_PER_ELEMENT,
          stepMode: "vertex",
          attributes: [
            {
              shaderLocation: 0,
              offset: 0 * Float32Array.BYTES_PER_ELEMENT,
              format: "float3"
            },
            {
              shaderLocation: 1,
              offset: 3 * Float32Array.BYTES_PER_ELEMENT,
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

  let depthTexture = device.createTexture({
    size: {
      width: window.width,
      height: window.height,
      depth: 1
    },
    arrayLayerCount: 1,
    mipLevelCount: 1,
    sampleCount: 1,
    dimension: "2d",
    format: "depth24plus-stencil8",
    usage: GPUTextureUsage.OUTPUT_ATTACHMENT
  });
  let depthStencilAttachment = {
    attachment: depthTexture.createView({ format: "depth24plus-stencil8" }),
    clearDepth: 1.0,
    depthLoadOp: "clear",
    depthStoreOp: "store",
    clearStencil: 0,
    stencilLoadOp: "clear",
    stencilStoreOp: "store",
  };
  window.onresize = e => {
    // recreate projection matrix
    mat4.identity(mProjection);
    mat4.perspective(mProjection, (2 * Math.PI) / 5, -(window.width / window.height), 0.1, 4096.0);
    // recreate depth texture
    depthTexture = device.createTexture({
      size: {
        width: window.width,
        height: window.height,
        depth: 1
      },
      arrayLayerCount: 1,
      mipLevelCount: 1,
      sampleCount: 1,
      dimension: "2d",
      format: "depth24plus-stencil8",
      usage: GPUTextureUsage.OUTPUT_ATTACHMENT
    });
    depthStencilAttachment.attachment = depthTexture.createView({
      format: "depth24plus-stencil8"
    });
  };

  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

    let now = Date.now() / 1e3;
    mat4.identity(mModel);
    mat4.rotateY(mModel, mModel, now);
    mat4.scale(mModel, mModel, vec3.fromValues(3, 3, 3));
    mat4.multiply(mModelViewProjection, mView, mModel);
    mat4.multiply(mModelViewProjection, mProjection, mModelViewProjection);
    stagedUniformBuffer.setSubData(0, mModelViewProjection);

    const backBufferView = swapChain.getCurrentTextureView();
    const commandEncoder = device.createCommandEncoder({});
    const renderPass = commandEncoder.beginRenderPass({
      colorAttachments: [{
        clearColor: { r: 0.125, g: 0.125, b: 0.125, a: 1.0 },
        loadOp: "clear",
        storeOp: "store",
        attachment: backBufferView
      }],
      depthStencilAttachment
    });
    renderPass.setPipeline(pipeline);
    renderPass.setBindGroup(0, uniformBindGroup);
    renderPass.setVertexBuffer(0, stagedVertexBuffer, 0);
    renderPass.setIndexBuffer(stagedIndexBuffer);
    renderPass.drawIndexed(modelIndices.length, 1, 0, 0, 0);
    renderPass.endPass();

    const commandBuffer = commandEncoder.finish();
    queue.submit([ commandBuffer ]);
    swapChain.present();
    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);

})();
