import WebGPU from "../index.js";

console.log(WebGPU.$getMemoryLayouts());

setInterval(() => {
  WebGPU.onFrame();
}, 1e3 / 60);
