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
    const [buffer, arrBuffer] = device.createBufferMapped({
      size: 128n,
      usage: 0x0001
    });
    console.log("Create Buffer Mapped: Buffer:", buffer);
    console.log("Create Buffer Mapped: ArrayBuffer", arrBuffer);
  }

  {
    const [buffer, arrBuffer] = await device.createBufferMappedAsync({
      size: 128n,
      usage: 0x0001
    });
    console.log("Create Buffer Mapped Async: Buffer:", buffer);
    console.log("Create Buffer Mapped Async: ArrayBuffer", arrBuffer);
  }

})();
