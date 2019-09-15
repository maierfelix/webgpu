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
    console.log(fence.getCompletedValue());
  }

  {
    const buffer = device.createBuffer({
      size: 128n,
      usage: 0x0001
    });
    await buffer.mapReadAsync();
    console.log(buffer);
  }

})();
