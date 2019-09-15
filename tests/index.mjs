import WebGPU from "../index.js";

console.log(WebGPU);

(async function main() {

  const adapter = await WebGPU.GPU.requestAdapter();
  console.log("Adapter Name:", adapter.name);
  console.log("Adapter Extensions:", adapter.extensions);

  const device = await adapter.requestDevice();
  console.log("Device Extensions:", device.extensions);

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
  }

})();
