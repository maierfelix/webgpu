#include "GPURayTracingAccelerationGeometry.h"
#include "GPUDevice.h"

#include "DescriptorDecoder.h"

#include <thread>
#include <chrono>
#include <cstdint>

Napi::FunctionReference GPURayTracingAccelerationGeometry::constructor;

GPURayTracingAccelerationGeometry::GPURayTracingAccelerationGeometry(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURayTracingAccelerationGeometry>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPURayTracingAccelerationGeometryDescriptor(device, info[1].As<Napi::Value>());
  this->instance = wgpuDeviceCreateRayTracingAccelerationGeometry(device->instance, &descriptor);
}

GPURayTracingAccelerationGeometry::~GPURayTracingAccelerationGeometry() {
  this->device.Reset();
  wgpuRayTracingAccelerationGeometryRelease(this->instance);
}

Napi::Object GPURayTracingAccelerationGeometry::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURayTracingAccelerationGeometry", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURayTracingAccelerationGeometry", func);
  return exports;
}
