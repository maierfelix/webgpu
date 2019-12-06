#include "GPUQueue.h"
#include "GPUDevice.h"
#include "GPUFence.h"
#include "GPUCommandBuffer.h"

#include <vector>

Napi::FunctionReference GPUQueue::constructor;

GPUQueue::GPUQueue(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUQueue>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  this->instance = wgpuDeviceCreateQueue(device->instance);
}

GPUQueue::~GPUQueue() {
  // destructor
}

Napi::Value GPUQueue::submit(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Napi::Array array = info[0].As<Napi::Array>();

  uint32_t length = array.Length();
  std::vector<WGPUCommandBuffer> commands;
  for (unsigned int ii = 0; ii < length; ++ii) {
    Napi::Object item = array.Get(ii).As<Napi::Object>();
    WGPUCommandBuffer value = Napi::ObjectWrap<GPUCommandBuffer>::Unwrap(item)->instance;
    commands.push_back(value);
  };

  wgpuQueueSubmit(this->instance, length, commands.data());

  return env.Undefined();
}

Napi::Value GPUQueue::createFence(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>()
  };
  if (info[0].IsObject()) {
    args.push_back(info[0].As<Napi::Value>());
  }
  Napi::Object fence = GPUFence::constructor.New(args);
  return fence;
}

Napi::Value GPUQueue::signal(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  WGPUFence fence = Napi::ObjectWrap<GPUFence>::Unwrap(info[0].ToObject())->instance;

  uint64_t signalValue = static_cast<uint64_t>(info[0].As<Napi::Number>().Uint32Value());
  wgpuQueueSignal(this->instance, fence, signalValue);

  return env.Undefined();
}

Napi::Object GPUQueue::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUQueue", {
    InstanceMethod(
      "submit",
      &GPUQueue::submit,
      napi_enumerable
    ),
    InstanceMethod(
      "createFence",
      &GPUQueue::createFence,
      napi_enumerable
    ),
    InstanceMethod(
      "signal",
      &GPUQueue::signal,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUQueue", func);
  return exports;
}
