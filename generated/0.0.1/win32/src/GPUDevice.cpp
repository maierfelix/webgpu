#include "GPUDevice.h"
#include "GPUAdapter.h"
#include "GPUQueue.h"
#include "GPUBuffer.h"
#include "GPUTexture.h"
#include "GPUSampler.h"
#include "GPUBindGroupLayout.h"
#include "GPUPipelineLayout.h"
#include "GPUBindGroup.h"
#include "GPUShaderModule.h"
#include "GPUComputePipeline.h"
#include "GPURenderPipeline.h"
#include "GPUCommandEncoder.h"
#include "GPURenderBundleEncoder.h"
#include "GPURayTracingAccelerationGeometry.h"
#include "GPURayTracingAccelerationInstance.h"

#include "WebGPUWindow.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUDevice::constructor;

GPUDevice::GPUDevice(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUDevice>(info) {
  Napi::Env env = info.Env();

  // expect arg 0 be GPUAdapter
  this->adapter.Reset(info[0].ToObject(), 1);
  this->_adapter = Napi::ObjectWrap<GPUAdapter>::Unwrap(this->adapter.Value())->instance;

  if (info[1].IsObject()) {
    Napi::Object obj = info[1].As<Napi::Object>();
    if (obj.Has(Napi::String::New(env, "extensions"))) {
      this->extensions.Reset(obj.Get("extensions").As<Napi::Object>(), 1);
    }
    if (obj.Has(Napi::String::New(env, "limits"))) {
      this->limits.Reset(obj.Get("limits").As<Napi::Object>(), 1);
    }
  }

  this->instance = this->_adapter.CreateDevice();
  this->binding = this->createBinding(info, this->instance);
  if (this->binding == nullptr) {
    Napi::Error::New(env, "Failed to create binding backend").ThrowAsJavaScriptException();
    return;
  }

  DawnProcTable procs = dawn_native::GetProcs();

  dawnProcSetProcs(&procs);
  procs.deviceSetUncapturedErrorCallback(
    this->instance,
    [](WGPUErrorType errorType, const char* message, void* devicePtr) {
      std::string type;
      switch (errorType) {
        case WGPUErrorType_Validation:
          type = "Validation";
        break;
        case WGPUErrorType_OutOfMemory:
          type = "Out of memory";
        break;
        case WGPUErrorType_Unknown:
          type = "Unknown";
        break;
        case WGPUErrorType_DeviceLost:
          type = "Device lost";
        break;
        default:
          type = "Undefined";
        break;
      }
      GPUDevice* self = reinterpret_cast<GPUDevice*>(devicePtr);
      Napi::Env env = self->onErrorCallback.Env();
      self->onErrorCallback.Call({
        Napi::String::New(env, type),
        Napi::String::New(env, (type + " Error: " + message))
      });
    },
    reinterpret_cast<void*>(this)
  );
  //this->device = wgpu::Device::Acquire(this->instance);
  this->mainQueue.Reset(this->createQueue(info), 1);
}

GPUDevice::~GPUDevice() {
  this->extensions.Reset();
  this->limits.Reset();
  this->adapter.Reset();
  this->mainQueue.Reset();
  this->onErrorCallback.Reset();

  delete this->binding;
  wgpuDeviceRelease(this->instance);
}

BackendBinding* GPUDevice::createBinding(const Napi::CallbackInfo& info, WGPUDevice device) {
  Napi::Env env = info.Env();

  GPUAdapter* adapter = Napi::ObjectWrap<GPUAdapter>::Unwrap(this->adapter.Value());
  WebGPUWindow* window = Napi::ObjectWrap<WebGPUWindow>::Unwrap(adapter->window.Value());

  dawn_native::BackendType backendType = adapter->instance.GetBackendType();
  BackendBinding* binding = CreateBinding(backendType, window->instance, device);

  return binding;
}

Napi::Object GPUDevice::createQueue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object queue = GPUQueue::constructor.New({
    info.This().As<Napi::Value>()
  });
  return queue;
}

Napi::Value GPUDevice::GetExtensions(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->extensions.IsEmpty()) return env.Null();
  return this->extensions.Value().As<Napi::Object>();
}

Napi::Value GPUDevice::GetLimits(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->extensions.IsEmpty()) return env.Null();
  return this->extensions.Value().As<Napi::Object>();
}

Napi::Value GPUDevice::GetAdapter(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return this->adapter.Value().As<Napi::Object>();
}

void GPUDevice::SetOnErrorCallback(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  this->onErrorCallback.Reset(value.As<Napi::Function>(), 1);
}

void GPUDevice::throwCallbackError(const Napi::Value& type, const Napi::Value& msg) {
  this->onErrorCallback.Call({ type, msg });
}

Napi::Value GPUDevice::tick(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  wgpuDeviceTick(this->instance);
  glfwPollEvents();
  return env.Undefined();
}

Napi::Value GPUDevice::createRayTracingAccelerationGeometry(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object accelerationGeometry = GPURayTracingAccelerationGeometry::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  return accelerationGeometry;
}

Napi::Value GPUDevice::createRayTracingAccelerationInstance(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object accelerationInstance = GPURayTracingAccelerationInstance::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  return accelerationInstance;
}

Napi::Value GPUDevice::createBuffer(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object buffer = GPUBuffer::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  return buffer;
}

Napi::Value GPUDevice::createBufferMapped(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object buffer = GPUBuffer::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  GPUBuffer* uwBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(buffer);
  Napi::Value arrBuffer = uwBuffer->mapReadAsync(info);
  Napi::Array out = Napi::Array::New(env);
  out.Set(Napi::Number::New(env, 0), buffer);
  out.Set(Napi::Number::New(env, 1), arrBuffer);
  return out;
}

Napi::Value GPUDevice::createBufferMappedAsync(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object buffer = GPUBuffer::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  Napi::Function callback = info[1].As<Napi::Function>();
  GPUBuffer* uwBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(buffer);
  Napi::Value arrBuffer = uwBuffer->mapReadAsync(info);
  Napi::Array out = Napi::Array::New(env);
  out.Set(Napi::Number::New(env, 0), buffer);
  out.Set(Napi::Number::New(env, 1), arrBuffer);
  callback.Call({ out });
  return env.Undefined();
}

Napi::Value GPUDevice::createTexture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object texture = GPUTexture::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  return texture;
}

Napi::Value GPUDevice::createSampler(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>()
  };
  if (info[0].IsObject()) args.push_back(info[0].As<Napi::Value>());
  Napi::Object sampler = GPUSampler::constructor.New(args);
  return sampler;
}

Napi::Value GPUDevice::createBindGroupLayout(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  };
  Napi::Object bindGroupLayout = GPUBindGroupLayout::constructor.New(args);
  return bindGroupLayout;
}

Napi::Value GPUDevice::createPipelineLayout(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  };
  Napi::Object pipelineLayout = GPUPipelineLayout::constructor.New(args);
  return pipelineLayout;
}

Napi::Value GPUDevice::createBindGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  };
  Napi::Object bindGroup = GPUBindGroup::constructor.New(args);
  return bindGroup;
}

Napi::Value GPUDevice::createShaderModule(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  };
  Napi::Object shaderModule = GPUShaderModule::constructor.New(args);
  return shaderModule;
}

Napi::Value GPUDevice::createComputePipeline(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  };
  Napi::Object computePipeline = GPUComputePipeline::constructor.New(args);
  return computePipeline;
}

Napi::Value GPUDevice::createRenderPipeline(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  };
  Napi::Object renderPipeline = GPURenderPipeline::constructor.New(args);
  return renderPipeline;
}

Napi::Value GPUDevice::createCommandEncoder(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>()
  };
  if (info[0].IsObject()) args.push_back(info[0].As<Napi::Value>());
  Napi::Object commandEncoder = GPUCommandEncoder::constructor.New(args);
  return commandEncoder;
}

Napi::Value GPUDevice::createRenderBundleEncoder(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  };
  Napi::Object renderBundleEncoder = GPURenderBundleEncoder::constructor.New(args);
  return renderBundleEncoder;
}

Napi::Value GPUDevice::getQueue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return this->mainQueue.Value().As<Napi::Object>();
}

Napi::Object GPUDevice::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUDevice", {
    InstanceAccessor(
      "extensions",
      &GPUDevice::GetExtensions,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "limits",
      &GPUDevice::GetLimits,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "adapter",
      &GPUDevice::GetAdapter,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "_onErrorCallback",
      nullptr,
      &GPUDevice::SetOnErrorCallback,
      napi_enumerable
    ),
    InstanceMethod(
      "getQueue",
      &GPUDevice::getQueue,
      napi_enumerable
    ),
    InstanceMethod(
      "tick",
      &GPUDevice::tick,
      napi_enumerable
    ),
    InstanceMethod(
      "createRayTracingAccelerationGeometry",
      &GPUDevice::createRayTracingAccelerationGeometry,
      napi_enumerable
    ),
    InstanceMethod(
      "createRayTracingAccelerationInstance",
      &GPUDevice::createRayTracingAccelerationInstance,
      napi_enumerable
    ),
    InstanceMethod(
      "createBuffer",
      &GPUDevice::createBuffer,
      napi_enumerable
    ),
    InstanceMethod(
      "createBufferMapped",
      &GPUDevice::createBufferMapped,
      napi_enumerable
    ),
    InstanceMethod(
      "_createBufferMappedAsync",
      &GPUDevice::createBufferMappedAsync,
      napi_enumerable
    ),
    InstanceMethod(
      "createTexture",
      &GPUDevice::createTexture,
      napi_enumerable
    ),
    InstanceMethod(
      "createSampler",
      &GPUDevice::createSampler,
      napi_enumerable
    ),
    InstanceMethod(
      "createBindGroupLayout",
      &GPUDevice::createBindGroupLayout,
      napi_enumerable
    ),
    InstanceMethod(
      "createPipelineLayout",
      &GPUDevice::createPipelineLayout,
      napi_enumerable
    ),
    InstanceMethod(
      "createBindGroup",
      &GPUDevice::createBindGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "createShaderModule",
      &GPUDevice::createShaderModule,
      napi_enumerable
    ),
    InstanceMethod(
      "createComputePipeline",
      &GPUDevice::createComputePipeline,
      napi_enumerable
    ),
    InstanceMethod(
      "createRenderPipeline",
      &GPUDevice::createRenderPipeline,
      napi_enumerable
    ),
    InstanceMethod(
      "createCommandEncoder",
      &GPUDevice::createCommandEncoder,
      napi_enumerable
    ),
    InstanceMethod(
      "createRenderBundleEncoder",
      &GPUDevice::createRenderBundleEncoder,
      napi_enumerable
    ),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUDevice", func);
  return exports;
}
