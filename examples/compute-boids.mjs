import WebGPU from "../index.js";

Object.assign(global, WebGPU);

const numParticles = 2000;

const vsSrc = `
  #version 450
  #pragma shader_stage(vertex)

  layout(location = 0) in vec2 a_particlePos;
  layout(location = 1) in vec2 a_particleVel;
  layout(location = 2) in vec2 a_pos;

  void main() {
    float angle = -atan(a_particleVel.x, a_particleVel.y);
    vec2 pos = vec2(
      a_pos.x * cos(angle) - a_pos.y * sin(angle),
      a_pos.x * sin(angle) + a_pos.y * cos(angle)
    );
    gl_Position = vec4(pos + a_particlePos, 0, 1);
  }
`;

const fsSrc = `
  #version 450
  #pragma shader_stage(fragment)

  layout(location = 0) out vec4 outColor;

  void main() {
    outColor = vec4(1.0);
  }
`;

const csSrc = `
  #version 450
  #pragma shader_stage(compute)

  struct Particle {
    vec2 pos;
    vec2 vel;
  };

  layout(std140, set = 0, binding = 0) uniform SimParams {
    float deltaT;
    float rule1Distance;
    float rule2Distance;
    float rule3Distance;
    float rule1Scale;
    float rule2Scale;
    float rule3Scale;
  } params;

  layout(std140, set = 0, binding = 1) buffer ParticlesA {
    Particle particles[${numParticles}];
  } particlesA;

  layout(std140, set = 0, binding = 2) buffer ParticlesB {
    Particle particles[${numParticles}];
  } particlesB;

  void main() {
    // https://github.com/austinEng/Project6-Vulkan-Flocking/blob/master/data/shaders/computeparticles/particle.comp
    uint index = gl_GlobalInvocationID.x;
    if (index >= ${numParticles}) { return; }
    vec2 vPos = particlesA.particles[index].pos;
    vec2 vVel = particlesA.particles[index].vel;
    vec2 cMass = vec2(0.0, 0.0);
    vec2 cVel = vec2(0.0, 0.0);
    vec2 colVel = vec2(0.0, 0.0);
    int cMassCount = 0;
    int cVelCount = 0;
    vec2 pos;
    vec2 vel;
    for (int i = 0; i < ${numParticles}; ++i) {
      if (i == index) { continue; }
      pos = particlesA.particles[i].pos.xy;
      vel = particlesA.particles[i].vel.xy;
      if (distance(pos, vPos) < params.rule1Distance) {
        cMass += pos;
        cMassCount++;
      }
      if (distance(pos, vPos) < params.rule2Distance) {
        colVel -= (pos - vPos);
      }
      if (distance(pos, vPos) < params.rule3Distance) {
        cVel += vel;
        cVelCount++;
      }
    }
    if (cMassCount > 0) {
      cMass = cMass / cMassCount - vPos;
    }
    if (cVelCount > 0) {
      cVel = cVel / cVelCount;
    }
    vVel += cMass * params.rule1Scale + colVel * params.rule2Scale + cVel * params.rule3Scale;
    // clamp velocity for a more pleasing simulation.
    vVel = normalize(vVel) * clamp(length(vVel), 0.0, 0.1);
    // kinematic update
    vPos += vVel * params.deltaT;
    // Wrap around boundary
    if (vPos.x < -1.0) vPos.x = 1.0;
    if (vPos.x > 1.0) vPos.x = -1.0;
    if (vPos.y < -1.0) vPos.y = 1.0;
    if (vPos.y > 1.0) vPos.y = -1.0;
    particlesB.particles[index].pos = vPos;
    // Write back
    particlesB.particles[index].vel = vVel;
  }
`;

(async function main() {

  const window = new WebGPUWindow({
    width: 640,
    height: 480,
    title: "WebGPU",
    resizable: false
  });

  const adapter = await GPU.requestAdapter({ window });

  const device = await adapter.requestDevice();

  const queue = device.getQueue();

  const context = window.getContext("webgpu");

  const swapChainFormat = await context.getSwapChainPreferredFormat(device);

  let swapChain = context.configureSwapChain({
    device: device,
    format: swapChainFormat
  });
  window.onresize = e => {
    swapChain.reconfigure({
      format: swapChainFormat
    });
  };

  const vertexShaderModule = device.createShaderModule({ code: vsSrc });
  const fragmentShaderModule = device.createShaderModule({ code: fsSrc });
  const computeShaderModule = device.createShaderModule({ code: csSrc });

  const renderPipeline = device.createRenderPipeline({
    layout: device.createPipelineLayout({ bindGroupLayouts: [] }),
    sampleCount: 1,
    vertexStage: {
      module: vertexShaderModule,
      entryPoint: "main"
    },
    fragmentStage: {
      module: fragmentShaderModule,
      entryPoint: "main"
    },
    primitiveTopology: "triangle-list",
    vertexState: {
      indexFormat: "uint32",
      vertexBuffers: [
        {
          arrayStride: 4 * Float32Array.BYTES_PER_ELEMENT,
          stepMode: "instance",
          attributes: [
            {
              shaderLocation: 0,
              offset: 0 * Float32Array.BYTES_PER_ELEMENT,
              format: "float2"
            },
            {
              shaderLocation: 1,
              offset: 2 * Float32Array.BYTES_PER_ELEMENT,
              format: "float2"
            }
          ]
        },
        {
          arrayStride: 2 * Float32Array.BYTES_PER_ELEMENT,
          stepMode: "vertex",
          attributes: [
            {
              shaderLocation: 2,
              offset: 0 * Float32Array.BYTES_PER_ELEMENT,
              format: "float2"
            }
          ]
        }
      ]
    },
    rasterizationState: {
      frontFace: "CCW",
      cullMode: "none"
    },
    colorStates: [{
      format: swapChainFormat,
      alphaBlend: {},
      colorBlend: {}
    }]
  });

  const computeBindGroupLayout = device.createBindGroupLayout({
    entries: [
      { binding: 0, visibility: GPUShaderStage.COMPUTE, type: "uniform-buffer" },
      { binding: 1, visibility: GPUShaderStage.COMPUTE, type: "storage-buffer" },
      { binding: 2, visibility: GPUShaderStage.COMPUTE, type: "storage-buffer" }
    ]
  });

  const computePipeline = device.createComputePipeline({
    layout: device.createPipelineLayout({ bindGroupLayouts: [ computeBindGroupLayout ] }),
    computeStage: {
      module: computeShaderModule,
      entryPoint: "main"
    }
  });

  const vertexBufferData = new Float32Array([-0.01, -0.02, 0.01, -0.02, 0.00, 0.02]);
  const verticesBuffer = device.createBuffer({
    size: vertexBufferData.byteLength,
    usage: GPUBufferUsage.VERTEX | GPUBufferUsage.COPY_DST,
  });
  verticesBuffer.setSubData(0, vertexBufferData);
  const simParamData = new Float32Array([0.04, 0.1, 0.025, 0.025, 0.02, 0.05, 0.005]);
  const simParamBuffer = device.createBuffer({
    size: simParamData.byteLength,
    usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
  });
  simParamBuffer.setSubData(0, simParamData);
  const initialParticleData = new Float32Array(numParticles * 4);
  for (let ii = 0; ii < numParticles; ++ii) {
    initialParticleData[4 * ii + 0] = 2 * (Math.random() - 0.5);
    initialParticleData[4 * ii + 1] = 2 * (Math.random() - 0.5);
    initialParticleData[4 * ii + 2] = 2 * (Math.random() - 0.5) * 0.1;
    initialParticleData[4 * ii + 3] = 2 * (Math.random() - 0.5) * 0.1;
  };
  const particleBuffers = new Array(2);
  const particleBindGroups = new Array(2);
  for (let ii = 0; ii < 2; ++ii) {
    particleBuffers[ii] = device.createBuffer({
      size: initialParticleData.byteLength,
      usage: GPUBufferUsage.COPY_DST | GPUBufferUsage.VERTEX | GPUBufferUsage.STORAGE
    });
    particleBuffers[ii].setSubData(0, initialParticleData);
  };
  for (let ii = 0; ii < 2; ++ii) {
    particleBindGroups[ii] = device.createBindGroup({
      layout: computeBindGroupLayout,
      entries: [{
        binding: 0,
        buffer: simParamBuffer,
        offset: 0,
        size: simParamData.byteLength
      }, {
        binding: 1,
        buffer: particleBuffers[ii],
        offset: 0,
        size: initialParticleData.byteLength
      }, {
        binding: 2,
        buffer: particleBuffers[(ii + 1) % 2],
        offset: 0,
        size: initialParticleData.byteLength
      }],
    });
  };

  let frames = 0;
  function onFrame() {
    if (!window.shouldClose()) setTimeout(onFrame, 1e3 / 60);

    const backBufferView = swapChain.getCurrentTextureView();

    const commandEncoder = device.createCommandEncoder({});
    {
      const passEncoder = commandEncoder.beginComputePass({});
      passEncoder.setPipeline(computePipeline);
      passEncoder.setBindGroup(0, particleBindGroups[frames % 2]);
      passEncoder.dispatch(numParticles);
      passEncoder.endPass();
    }

    {
      const passEncoder = commandEncoder.beginRenderPass({
        colorAttachments: [{
          clearColor: { r: 0.0, g: 0.0, b: 0.0, a: 1.0 },
          loadOp: "clear",
          storeOp: "store",
          attachment: backBufferView
        }]
      });
      passEncoder.setPipeline(renderPipeline);
      passEncoder.setVertexBuffer(0, particleBuffers[(frames + 1) % 2], 0);
      passEncoder.setVertexBuffer(1, verticesBuffer, 0);
      passEncoder.draw(3, numParticles, 0, 0);
      passEncoder.endPass();
    }
    queue.submit([ commandEncoder.finish() ]);

    swapChain.present();
    window.pollEvents();
    frames++;
  };
  setTimeout(onFrame, 1e3 / 60);

})();
