#include "GPUFence.h"
#include "GPUQueue.h"
#include "GPUDevice.h"

#include "DescriptorDecoder.h"

#include <thread>
#include <chrono>

Napi::FunctionReference GPUFence::constructor;

GPUFence::GPUFence(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUFence>(info) {
  Napi::Env env = info.Env();

  this->queue.Reset(info[0].As<Napi::Object>(), 1);
  GPUQueue* queue = Napi::ObjectWrap<GPUQueue>::Unwrap(this->queue.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(queue->device.Value());

  auto descriptor = DescriptorDecoder::GPUFenceDescriptor(device, info[1].As<Napi::Value>());

  this->instance = dawnQueueCreateFence(queue->instance, &descriptor);
}

GPUFence::~GPUFence() {
  // destructor
}

Napi::Value GPUFence::getCompletedValue(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  uint64_t completedValue = dawnFenceGetCompletedValue(this->instance);
  return Napi::BigInt::New(env, completedValue);
}

Napi::Value GPUFence::onCompletion(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  bool lossless;
  uint64_t completionValue = info[0].As<Napi::BigInt>().Uint64Value(&lossless);

  Napi::Function callback = info[1].As<Napi::Function>();

  dawnFenceOnCompletion(
    this->instance,
    static_cast<unsigned long long>(completionValue),
    [](DawnFenceCompletionStatus status, void* userdata) {

    },
    nullptr
  );

  GPUQueue* queue = Napi::ObjectWrap<GPUQueue>::Unwrap(this->queue.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(queue->device.Value());
  DawnDevice backendDevice = device->instance;

  dawnDeviceTick(backendDevice);
  if (dawnFenceGetCompletedValue(this->instance) != completionValue) {
    while (dawnFenceGetCompletedValue(this->instance) != completionValue) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      dawnDeviceTick(backendDevice);
    };
  }

  callback.Call({ });

  return env.Undefined();
}

Napi::Object GPUFence::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUFence", {
    InstanceMethod(
      "getCompletedValue",
      &GPUFence::getCompletedValue,
      napi_enumerable
    ),
    InstanceMethod(
      "_onCompletion",
      &GPUFence::onCompletion,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUFence", func);
  return exports;
}
