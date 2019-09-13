import WebGPU from "../index.js";

setInterval(() => {
  WebGPU.onFrame();
}, 1e3 / 60);
