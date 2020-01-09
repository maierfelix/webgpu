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

Napi::Value GPURayTracingShaderBindingTable::getOffset(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  WGPUShaderStageFlags shaderStage = static_cast<WGPUShaderStageFlags>(
    info[0].As<Napi::Number>().Uint32Value()
  );

  uint32_t offset = wgpuRayTracingShaderBindingTableGetOffset(this->instance, shaderStage);

  return Napi::Number::New(env, offset);
}

Napi::Value GPURayTracingShaderBindingTable::destroy(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  wgpuRayTracingShaderBindingTableDestroy(this->instance);
  return env.Undefined();
}

Napi::Object GPURayTracingShaderBindingTable::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURayTracingShaderBindingTable", {
    InstanceMethod(
      "getOffset",
      &GPURayTracingShaderBindingTable::getOffset,
      napi_enumerable
    ),
    InstanceMethod(
      "destroy",
      &GPURayTracingShaderBindingTable::destroy,
      napi_enumerable
    ),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURayTracingShaderBindingTable", func);
  return exports;
}
