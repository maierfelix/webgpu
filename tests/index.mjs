import WebGPU from "../index.js";

console.log(WebGPU);

//WebGPU.runExample();

(async function main() {

  const adapter = await WebGPU.GPU.requestAdapter();
  console.log("Adapter Name:", adapter.name);
  console.log("Adapter Extensions:", adapter.extensions);

  const device = await adapter.requestDevice();
  console.log("Device Extensions:", device.extensions);

})();

/*setInterval(() => {
  WebGPU.onFrame();
}, 1e3 / 60);*/
