import WebGPU from "../index.js";

(async function main() {

  const adapter = await WebGPU.GPU.requestAdapter();
  console.log("Adapter:", adapter);
  console.log("Adapter Name:", adapter.name);
  console.log("Adapter Extensions:", adapter.extensions);

  const device = await adapter.requestDevice();
  console.log("Device:", device);

  console.log(device);
  const queue = device.getQueue();
  console.log("Queue:", queue);

  {
    const fence = queue.createFence();
    queue.signal(fence, 2n);
    await fence.onCompletion(2n);
    console.log("Fence Completed Value:", fence.getCompletedValue());
  }

  {
    const buffer = device.createBuffer({
      size: 128n,
      usage: 0x0001
    });
    console.log("Buffer:", buffer);
    let readBuffer = await buffer.mapReadAsync();
    console.log("Read Buffer:", readBuffer);
  }

  {
    const buffer = device.createBuffer({
      size: 128n,
      usage: 0x0002
    });
    console.log("Buffer:", buffer);
    let writeBuffer = await buffer.mapWriteAsync();
    console.log("Write Buffer:", writeBuffer);
    buffer.unmap();
    console.log("Unmapped Buffer:", writeBuffer);
    buffer.destroy();
    console.log("Destroyed Buffer:", writeBuffer);
  }

  {
    const [buffer, arrayBuffer] = device.createBufferMapped({
      size: 128n,
      usage: 0x0001
    });
    console.log("Create Buffer Mapped: Buffer:", buffer);
    console.log("Create Buffer Mapped: ArrayBuffer", arrayBuffer);
  }

  {
    const [buffer, arrayBuffer] = await device.createBufferMappedAsync({
      size: 128n,
      usage: 0x0001
    });
    console.log("Create Buffer Mapped Async: Buffer:", buffer);
    console.log("Create Buffer Mapped Async: ArrayBuffer", arrayBuffer);
  }

  {
    let image = {
      width: 256,
      height: 256,
      data: new Uint8Array(256 * 256 * 4)
    };

    let texture = device.createTexture({
      size: {
        width: image.width,
        height: image.height,
        depth: 1
      },
      arrayLayerCount: 1,
      mipLevelCount: 1,
      sampleCount: 1,
      dimension: "2d",
      format: "rgba8unorm",
      usage: 0x00000002 | 0x00000004
    });
    console.log("Texture:", texture);

    let [textureDataBuffer, textureArrayBuffer] = device.createBufferMapped({
      size: BigInt(image.data.length),
      usage: 0x00000001
    });
    console.log("Texture Buffer:", textureDataBuffer);
    console.log("Texture ArrayBuffer:", textureArrayBuffer);

    new Uint8Array(textureArrayBuffer).set(image.data, 0x0);
    textureDataBuffer.unmap();

    let textureView = texture.createView();
    console.log("Texture View:", textureView);

  }

  let sampler = device.createSampler({ });
  console.log("Sampler:", sampler);

  let bindGroupLayout = device.createBindGroupLayout({ 
    bindings: [
      {
        binding: 0,
        visibility: 0x00000002,
        type: "sampler"
      }
    ] 
  });
  console.log("Bind Group Layout:", bindGroupLayout);

  let pipelineLayout = device.createPipelineLayout({
    bindGroupLayouts: [bindGroupLayout]
  });
  console.log("Pipeline Layout:", pipelineLayout);

  let vertexShaderSource = `
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

  let fragmentShaderSource = `
    #version 450
    #pragma shader_stage(fragment)
    layout(location = 0) out vec4 fragColor;
    void main() {
      fragColor = vec4(mix(vec3(0.25), vec3(0.5), 0.5), 1.0);
    }
  `;

  let vertexShaderModule = device.createShaderModule({
    code: vertexShaderSource
  });
  console.log("Vertex Shader Module:", vertexShaderModule);

  let fragmentShaderModule = device.createShaderModule({
    code: fragmentShaderSource
  });
  console.log("Fragment Shader Module:", fragmentShaderModule);

  // TODO: to hex
  let colorState = {
    format: "bgra8unorm",
    alphaBlend: {
      srcFactor: "src-alpha",
      dstFactor: "one-minus-src-alpha",
      operation: "add"
    },
    colorBlend: {
      srcFactor: "src-alpha",
      dstFactor: "one-minus-src-alpha",
      operation: "add"
    },
    writeMask: 0x0000000F
  };

  let vertexStageDescriptor = {
    module: vertexShaderModule,
    entryPoint: "main"
  };
  let fragmentStageDescriptor = {
    module: fragmentShaderModule,
    entryPoint: "main"
  };

  let positionAttribute = {
    shaderLocation: 0,
    offset: 0n,
    format: "float4"
  };

  let vertexBufferDescriptor = {
    stride: BigInt(8 * 4),
    attributes: [positionAttribute]
  };

  let vertexInputDescriptor = {
    buffers: [vertexBufferDescriptor]
  };

  let pipeline = device.createRenderPipeline({
    layout: pipelineLayout,
    vertexStage: vertexStageDescriptor,
    fragmentStage: fragmentStageDescriptor,
    primitiveTopology: "triangle-list",
    colorStates: [colorState],
    vertexInput: vertexInputDescriptor
  });
  console.log("Render Pipeline:", pipeline);

  return;

  let swapChainDescriptor = {
    device: device,
    format: "bgra8unorm"
  };

  let swapchain = gpu.configureSwapChain(swapChainDescriptor);

  let swapchainTexture = swapchain.getCurrentTexture();
  let renderAttachment = swapchain.createDefaultView();

})();
