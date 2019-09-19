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
    layout(location = 0) in vec4 position;
    void main() {
      gl_Position = vec4(position);
    }
  `;

  let fragmentShaderSource = `
    #version 450
    #pragma shader_stage(fragment)
    layout(location = 0) out vec4 fragColor;
    void main() {
      fragColor = vec4(1, 0, 0, 1);
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

  let vertexDataBufferDescriptor = {
    size: BigInt((4 * 4) * 3),
    usage: 0x00000001 | 0x00000002
  };
  let vertexBuffer = device.createBuffer(vertexDataBufferDescriptor);
  console.log("Vertex Buffer:", vertexBuffer);

  let vertexArrayBuffer = await vertexBuffer.mapWriteAsync();
  console.log("Vertex Buffer Map Wirte:", vertexArrayBuffer);

  let vertexWriteArray = new Float32Array(vertexArrayBuffer);
  vertexWriteArray.set(new Float32Array([
    0, 0.8, 0, 1,
    -0.8, -0.8, 0, 1,
    0.8, -0.8, 0, 1
  ]));
  vertexBuffer.unmap();

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
    stride: BigInt(4 * 4),
    attributes: [positionAttribute]
  };

  let vertexInputDescriptor = {
    buffers: [vertexBufferDescriptor]
  };

  let renderPipeline = device.createRenderPipeline({
    layout: pipelineLayout,
    vertexStage: vertexStageDescriptor,
    fragmentStage: fragmentStageDescriptor,
    primitiveTopology: "triangle-list",
    colorStates: [colorState],
    vertexInput: vertexInputDescriptor
  });
  console.log("Render Pipeline:", renderPipeline);

  let swapChainDescriptor = {
    device: device,
    format: "bgra8unorm"
  };

  let context = WebGPU.GPU.getContext("webgpu");
  console.log("Canvas Context:", context);

  let swapchain = context.configureSwapChain(swapChainDescriptor);
  console.log("Swapchain:", swapchain);

  let swapchainTexture = swapchain.getCurrentTexture();
  console.log("Swapchain Texture:", swapchainTexture);

  let renderAttachment = swapchainTexture.createView();
  console.log("Render Attachment:", renderAttachment);

  let darkBlue = {
    r: 0.15,
    g: 0.15,
    b: 0.5,
    a: 1
  };

  let colorAttachmentDescriptor = {
    attachment: renderAttachment,
    loadOp: "clear",
    storeOp: "store",
    clearColor: darkBlue
  };

  let renderPassDescriptor = {
    colorAttachments: [colorAttachmentDescriptor]
  };

  let commandEncoder = device.createCommandEncoder();
  console.log("Command Encoder:", commandEncoder);

  let renderPassEncoder = commandEncoder.beginRenderPass(renderPassDescriptor);
  console.log("RenderPass Encoder:", renderPassEncoder);

  renderPassEncoder.setPipeline(renderPipeline);
  renderPassEncoder.setVertexBuffers(0, [vertexBuffer], [0n]);
  renderPassEncoder.draw(3, 1, 0, 0);
  renderPassEncoder.endPass();

  let commandBuffer = commandEncoder.finish();

  queue.submit([commandBuffer]);

})();
