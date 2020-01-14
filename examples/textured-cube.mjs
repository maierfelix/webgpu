import fs from "fs";
import WebGPU from "../index.js";
import glMatrix from "gl-matrix";
import lodepng from "@cwasm/lodepng";

Object.assign(global, WebGPU);
Object.assign(global, glMatrix);

const vsSrc = `
  #version 450
  #pragma shader_stage(vertex)

  layout(set = 0, binding = 0) uniform CameraUniform {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
  };

  layout(location = 0) in vec3 position;
  layout(location = 1) in vec3 normal;
  layout(location = 2) in vec2 uv;

  layout(location = 0) out vec3 vNormal;
  layout(location = 1) out vec2 vUV;
  layout(location = 2) out vec4 vWorldPosition;

  void main() {
    mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
    gl_Position = MVP * vec4(position, 1.0);
    vNormal = mat3(modelMatrix) * normal;
    vUV = uv * 0.5;
    vWorldPosition = modelMatrix * vec4(position, 1.0);
  }
`;

const fsSrc = `
  #version 450
  #pragma shader_stage(fragment)

  layout(set = 0, binding = 0) uniform CameraUniform {
    mat4 modelMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
  };

  layout(set = 0, binding = 1) uniform sampler mySampler;
  layout(set = 0, binding = 2) uniform texture2D myTexture;

  layout(location = 0) in vec3 vNormal;
  layout(location = 1) in vec2 vUV;
  layout(location = 2) in vec4 vWorldPosition;

  layout(location = 0) out vec4 outColor;

  void main() {
    vec3 color = texture(sampler2D(myTexture, mySampler), vUV).rgb;

    vec3 cameraPosition = (viewMatrix * vWorldPosition).xyz;
    vec3 lightPosition = vec3(1, 6, -2) - vWorldPosition.xyz;

    vec3 N = normalize(vNormal);
    vec3 V = normalize(cameraPosition);
    vec3 L = normalize(lightPosition);

    vec3 ambient = color * 0.1;
    vec3 diffuse = max(dot(L, N), 0.0) * color;
    outColor = vec4(ambient + diffuse, 1.0);
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

  const modelNormals = new Float32Array([
    // Front
    0.0,  0.0,  1.0,
    0.0,  0.0,  1.0,
    0.0,  0.0,  1.0,
    0.0,  0.0,  1.0,
    // Back
    0.0,  0.0, -1.0,
    0.0,  0.0, -1.0,
    0.0,  0.0, -1.0,
    0.0,  0.0, -1.0,
    // Top
    0.0,  1.0,  0.0,
    0.0,  1.0,  0.0,
    0.0,  1.0,  0.0,
    0.0,  1.0,  0.0,
    // Bottom
    0.0, -1.0,  0.0,
    0.0, -1.0,  0.0,
    0.0, -1.0,  0.0,
    0.0, -1.0,  0.0,
    // Right
    1.0,  0.0,  0.0,
    1.0,  0.0,  0.0,
    1.0,  0.0,  0.0,
    1.0,  0.0,  0.0,
    // Left
    -1.0,  0.0,  0.0,
    -1.0,  0.0,  0.0,
    -1.0,  0.0,  0.0,
    -1.0,  0.0,  0.0
  ]);

  const modelUVs = new Float32Array([
    // Front
    0.025,  0.01,
    0.175,  0.01,
    0.175,  0.175,
    0.025,  0.175,
    // Back
    0.025,  0.01,
    0.175,  0.01,
    0.175,  0.175,
    0.025,  0.175,
    // Top
    1.0,  0.0,
    1.0, -1.0,
    0.0, -1.0,
    0.0,  0.0,
    // Bottom
    0.0,  0.0,
    1.0,  0.0,
    1.0, -1.0,
    0.0, -1.0,
    // Right
    0.0,  0.0,
    -1.0,  0.0,
    -1.0, -1.0,
    0.0, -1.0,
    // Left
    1.0,  0.0,
    1.0, -1.0,
    0.0, -1.0,
    0.0,  0.0
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

  const img = lodepng.decode(fs.readFileSync("examples/assets/grass-block.png"));

  const window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU",
    resizable: false
  });

  const aspect = Math.abs(window.width / window.height);

  const mModel = mat4.create();
  const mView = mat4.create();
  const mProjection = mat4.create();

  mat4.perspective(mProjection, 45.0 * Math.PI / 180, -aspect, 0.1, 4096.0);
  mat4.lookAt(
    mView,
    vec3.fromValues(4, 4, -4),
    vec3.fromValues(0.0, 0.0, 0.0),
    vec3.fromValues(0.0, 0.0, 1.0)
  );

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

  const stagedNormalBuffer = device.createBuffer({
    size: modelNormals.byteLength,
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
  });
  stagedNormalBuffer.setSubData(0, modelNormals);

  const stagedUVBuffer = device.createBuffer({
    size: modelUVs.byteLength,
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST
  });
  stagedUVBuffer.setSubData(0, modelUVs);

  const stagedIndexBuffer = device.createBuffer({
    size: modelIndices.byteLength,
    usage: GPUBufferUsage.INDEX | GPUBufferUsage.COPY_DST
  });
  stagedIndexBuffer.setSubData(0, modelIndices);

  const stagedUniformBuffer = device.createBuffer({
    size: 48 * Float32Array.BYTES_PER_ELEMENT,
    usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST
  });
  stagedUniformBuffer.setSubData(0, mModel);
  stagedUniformBuffer.setSubData(16 * Float32Array.BYTES_PER_ELEMENT, mView);
  stagedUniformBuffer.setSubData(32 * Float32Array.BYTES_PER_ELEMENT, mProjection);

  const texture = device.createTexture({
    size: {
      width: img.width,
      height: img.height,
      depth: 1,
    },
    arrayLayerCount: 1,
    mipLevelCount: 1,
    sampleCount: 1,
    dimension: "2d",
    format: "rgba8unorm",
    usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.SAMPLED
  });

  // copy texture over to GPU
  {
    const rowPitch = Math.ceil(img.width * 4 / 256) * 256;

    let data = null;

    if (rowPitch == img.width * 4) {
      data = img.data;
    } else {
      data = new Uint8Array(rowPitch * img.height);
      for (let y = 0; y < window.height; ++y) {
        for (let x = 0; x < window.width; ++x) {
          let i = x * 4 + y * rowPitch;
          data[i] = img.data[i];
          data[i + 1] = img.data[i + 1];
          data[i + 2] = img.data[i + 2];
          data[i + 3] = img.data[i + 3];
        };
      };
    }

    const textureDataBuffer = device.createBuffer({
      size: data.byteLength,
      usage: GPUBufferUsage.COPY_SRC | GPUBufferUsage.COPY_DST,
    });
    textureDataBuffer.setSubData(0, data);

    const commandEncoder = device.createCommandEncoder({});
    commandEncoder.copyBufferToTexture({
      buffer: textureDataBuffer,
      rowPitch: rowPitch,
      arrayLayer: 0,
      mipLevel: 0,
      imageHeight: 0
    }, {
      texture: texture,
      mipLevel: 0,
      arrayLayer: 0,
      origin: { x: 0, y: 0, z: 0 }
    }, {
      width: img.width,
      height: img.height,
      depth: 1
    });
    queue.submit([commandEncoder.finish()]);
  }

  const sampler = device.createSampler({
    magFilter: "nearest",
    minFilter: "nearest",
    addressModeU: "mirror-repeat",
    addressModeV: "mirror-repeat",
    addressModeW: "mirror-repeat"
  });

  const uniformBindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.VERTEX | GPUShaderStage.FRAGMENT,
        type: "uniform-buffer",
        textureDimension: "2D"
      },
      {
        binding: 1,
        visibility: GPUShaderStage.FRAGMENT,
        type: "sampler"
      },
      {
        binding: 2,
        visibility: GPUShaderStage.FRAGMENT,
        type: "sampled-texture"
      }
    ]
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
          arrayStride: 3 * Float32Array.BYTES_PER_ELEMENT,
          stepMode: "vertex",
          attributes: [
            {
              shaderLocation: 0,
              offset: 0 * Float32Array.BYTES_PER_ELEMENT,
              format: "float3"
            }
          ]
        },
        {
          arrayStride: 3 * Float32Array.BYTES_PER_ELEMENT,
          stepMode: "vertex",
          attributes: [
            {
              shaderLocation: 1,
              offset: 0 * Float32Array.BYTES_PER_ELEMENT,
              format: "float3"
            }
          ]
        },
        {
          arrayStride: 2 * Float32Array.BYTES_PER_ELEMENT,
          stepMode: "vertex",
          attributes: [
            {
              shaderLocation: 2,
              offset: 0 * Float32Array.BYTES_PER_ELEMENT,
              format: "float2"
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
    bindings: [
      {
        binding: 0,
        buffer: stagedUniformBuffer,
        offset: 0,
        size: 48 * Float32Array.BYTES_PER_ELEMENT
      },
      {
        binding: 1,
        sampler: sampler,
        size: 0
      }, {
        binding: 2,
        textureView: texture.createView({ format: "rgba8unorm" }),
        size: 0
      }
    ]
  });

  const depthTexture = device.createTexture({
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

  const depthStencilAttachment = {
    attachment: depthTexture.createView({ format: "depth24plus-stencil8" }),
    clearDepth: 1.0,
    depthLoadOp: "clear",
    depthStoreOp: "store",
    clearStencil: 0,
    stencilLoadOp: "clear",
    stencilStoreOp: "store",
  };

  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

    let now = Date.now() / 1e3;
    mat4.identity(mModel);
    mat4.rotate(
      mModel,
      mModel,
      now * (90 * Math.PI / 180),
      vec3.fromValues(0.0, 0.0, 1.0)
    );
    stagedUniformBuffer.setSubData(0, mModel);

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
    renderPass.setVertexBuffer(1, stagedNormalBuffer, 0);
    renderPass.setVertexBuffer(2, stagedUVBuffer, 0);
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
