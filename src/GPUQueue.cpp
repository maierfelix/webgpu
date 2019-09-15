#include "GPUQueue.h"
#include "GPUDevice.h"
#include "GPUFence.h"

Napi::FunctionReference GPUQueue::constructor;

GPUQueue::GPUQueue(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUQueue>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  DawnDevice backendDevice = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value())->backendDevice;

  this->queue = dawnDeviceCreateQueue(backendDevice);
}

GPUQueue::~GPUQueue() {
  // destructor
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

  DawnQueue queue = this->queue;
  DawnFence fence = Napi::ObjectWrap<GPUFence>::Unwrap(info[0].ToObject())->fence;
  bool lossless;
  uint64_t signalValue = info[1].As<Napi::BigInt>().Uint64Value(&lossless);
  dawnQueueSignal(queue, fence, signalValue);

  return env.Undefined();
}

Napi::Object GPUQueue::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUQueue", {
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
