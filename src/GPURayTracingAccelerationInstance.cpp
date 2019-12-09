#include "GPURayTracingAccelerationInstance.h"
#include "GPUDevice.h"

#include "DescriptorDecoder.h"

#include <thread>
#include <chrono>
#include <cstdint>

Napi::FunctionReference GPURayTracingAccelerationInstance::constructor;

GPURayTracingAccelerationInstance::GPURayTracingAccelerationInstance(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURayTracingAccelerationInstance>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPURayTracingAccelerationInstanceDescriptor(device, info[1].As<Napi::Value>());
  this->instance = wgpuDeviceCreateRayTracingAccelerationInstance(device->instance, &descriptor);
}

GPURayTracingAccelerationInstance::~GPURayTracingAccelerationInstance() {
  this->device.Reset();
  wgpuRayTracingAccelerationInstanceRelease(this->instance);
}

Napi::Object GPURayTracingAccelerationInstance::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURayTracingAccelerationInstance", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURayTracingAccelerationInstance", func);
  return exports;
}
