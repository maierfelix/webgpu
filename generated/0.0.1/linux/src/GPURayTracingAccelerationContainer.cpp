#include "GPURayTracingAccelerationContainer.h"
#include "GPUDevice.h"

#include "DescriptorDecoder.h"

#include <thread>
#include <chrono>
#include <cstdint>

Napi::FunctionReference GPURayTracingAccelerationContainer::constructor;

GPURayTracingAccelerationContainer::GPURayTracingAccelerationContainer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURayTracingAccelerationContainer>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPURayTracingAccelerationContainerDescriptor(device, info[1].As<Napi::Value>());
  this->instance = wgpuDeviceCreateRayTracingAccelerationContainer(device->instance, &descriptor);
}

GPURayTracingAccelerationContainer::~GPURayTracingAccelerationContainer() {
  this->device.Reset();
  wgpuRayTracingAccelerationContainerRelease(this->instance);
}

Napi::Value GPURayTracingAccelerationContainer::destroy(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  wgpuRayTracingAccelerationContainerDestroy(this->instance);
  return env.Undefined();
}

Napi::Value GPURayTracingAccelerationContainer::getHandle(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  uint64_t handle = wgpuRayTracingAccelerationContainerGetHandle(this->instance);
  return Napi::BigInt::New(env, handle);
}

Napi::Object GPURayTracingAccelerationContainer::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURayTracingAccelerationContainer", {
    InstanceMethod(
      "destroy",
      &GPURayTracingAccelerationContainer::destroy,
      napi_enumerable
    ),
    InstanceMethod(
      "getHandle",
      &GPURayTracingAccelerationContainer::getHandle,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURayTracingAccelerationContainer", func);
  return exports;
}
