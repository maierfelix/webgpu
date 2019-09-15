import WebGPU from "../index.js";

console.log(WebGPU);

//WebGPU.runExample();

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

})();
