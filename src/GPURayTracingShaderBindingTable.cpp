#include "GPURayTracingShaderBindingTable.h"
#include "GPUDevice.h"

#include "DescriptorDecoder.h"

#include <thread>
#include <chrono>
#include <cstdint>

Napi::FunctionReference GPURayTracingShaderBindingTable::constructor;

GPURayTracingShaderBindingTable::GPURayTracingShaderBindingTable(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURayTracingShaderBindingTable>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPURayTracingShaderBindingTableDescriptor(device, info[1].As<Napi::Value>());
  this->instance = wgpuDeviceCreateRayTracingShaderBindingTable(device->instance, &descriptor);
}

GPURayTracingShaderBindingTable::~GPURayTracingShaderBindingTable() {
  this->device.Reset();
  wgpuRayTracingShaderBindingTableRelease(this->instance);
}

Napi::Object GPURayTracingShaderBindingTable::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURayTracingShaderBindingTable", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURayTracingShaderBindingTable", func);
  return exports;
}
