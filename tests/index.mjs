import WebGPU from "../index.js";

console.log(WebGPU);

(async function main() {

  const adapter = await WebGPU.GPU.requestAdapter();
  console.log("Adapter Name:", adapter.name);
  console.log("Adapter Extensions:", adapter.extensions);

  const device = await adapter.requestDevice();

})();

/*
const device = adapter.requestDevice();
console.log(device);
*/

/*setInterval(() => {
  WebGPU.onFrame();
}, 1e3 / 60);*/
