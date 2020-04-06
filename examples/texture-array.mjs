import WebGPU from "../index.js";

Object.assign(global, WebGPU);

const vsSrc = `
  #version 450
  #pragma shader_stage(vertex)
  layout (location = 0) out vec2 vUV;
  void main(void) {
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0 + -1.0f, 0.0, 1.0);
    vUV = vec2(uv.x, 1.0 - uv.y);
  }
`;

const fsSrc = `
  #version 450
  #pragma shader_stage(fragment)
  layout (location = 0) in vec2 vUv;
  layout (location = 0) out vec4 outColor;
  layout (binding = 0) uniform sampler TextureSampler;
  layout (binding = 1) uniform texture2DArray TextureArray;
  void main() {
    float split = 1.0 / 4.0;
    uint textureIndex = (
      vUv.x <= 1.0 * split ? 0 :
      vUv.x <= 2.0 * split ? 1 :
      vUv.x <= 3.0 * split ? 2 :
      vUv.x <= 4.0 * split ? 3 :
      0
    );
    outColor = texture(sampler2DArray(TextureArray, TextureSampler), vec3(vUv, textureIndex));
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

  const imageCount = 4;
  const imageWidth = 32;
  const imageHeight = 32;

  const textureArray = device.createTexture({
    size: {
      width: imageWidth,
      height: imageHeight,
      depth: 1
    },
    arrayLayerCount: imageCount,
    mipLevelCount: 1,
    sampleCount: 1,
    dimension: "2d",
    format: "rgba8unorm-srgb",
    usage: GPUTextureUsage.COPY_DST | GPUTextureUsage.SAMPLED
  });

  const textureArrayView = textureArray.createView({
    dimension: "2d-array",
    baseArrayLayer: 0,
    arrayLayerCount: imageCount,
    format: "rgba8unorm-srgb"
  });

  const textureSampler = device.createSampler({
    magFilter: "linear",
    minFilter: "linear",
    addressModeU: "repeat",
    addressModeV: "repeat",
    addressModeW: "repeat"
  });

  const imageColors = [
    [255, 0, 0],   // red
    [0, 255, 0],   // green
    [0, 0, 255],   // blue
    [255, 0, 255], // pink
  ];

  const commandEncoder = device.createCommandEncoder({});
  for (let ii = 0; ii < imageCount; ++ii) {
    const rowPitch = Math.ceil(imageWidth * 4 / 256) * 256;
    const data = new Uint8Array(rowPitch * imageHeight);
    for (let yy = 0; yy < imageHeight; ++yy) {
      for (let xx = 0; xx < imageWidth; ++xx) {
        const index = xx * 4 + yy * rowPitch;
        data[index + 0] = imageColors[ii][0];
        data[index + 1] = imageColors[ii][1];
        data[index + 2] = imageColors[ii][2];
        data[index + 3] = 255;
      };
    };

    const textureBuffer = device.createBuffer({
      size: data.byteLength,
      usage: GPUBufferUsage.COPY_SRC | GPUBufferUsage.COPY_DST
    });
    textureBuffer.setSubData(0, data);

    commandEncoder.copyBufferToTexture(
      {
        buffer: textureBuffer,
        rowPitch: rowPitch,
        arrayLayer: 0,
        mipLevel: 0,
        imageHeight: 0
      },
      {
        texture: textureArray,
        mipLevel: 0,
        arrayLayer: ii,
        origin: { x: 0, y: 0, z: 0 }
      },
      {
        width: imageWidth,
        height: imageHeight,
        depth: 1
      }
    );
  };
  queue.submit([ commandEncoder.finish() ]);

  const bindGroupLayout = device.createBindGroupLayout({
    bindings: [
      {
        binding: 0,
        visibility: GPUShaderStage.FRAGMENT,
        type: "sampler"
      },
      {
        binding: 1,
        visibility: GPUShaderStage.FRAGMENT,
        type: "sampled-texture",
        textureDimension: "2d-array"
      }
    ]
  });

  const bindGroup = device.createBindGroup({
    layout: bindGroupLayout,
    bindings: [
      {
        binding: 0,
        sampler: textureSampler,
        size: 0
      },
      {
        binding: 1,
        textureView: textureArrayView,
        size: 0
      }
    ]
  });

  const pipeline = device.createRenderPipeline({
    layout: device.createPipelineLayout({
      bindGroupLayouts: [bindGroupLayout]
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
      }
    ]
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
    renderPass.setBindGroup(0, bindGroup);
    renderPass.draw(3, 1, 0, 0);
    renderPass.endPass();

    const commandBuffer = commandEncoder.finish();
    queue.submit([ commandBuffer ]);
    swapChain.present();
    window.pollEvents();
  };
  setTimeout(onFrame, 1e3 / 60);

})();
