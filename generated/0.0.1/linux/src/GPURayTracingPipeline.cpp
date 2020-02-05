#include "GPURayTracingPipeline.h"
#include "GPUDevice.h"
#include "GPUShaderModule.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPURayTracingPipeline::constructor;

GPURayTracingPipeline::GPURayTracingPipeline(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURayTracingPipeline>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPURayTracingPipelineDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuDeviceCreateRayTracingPipeline(device->instance, &descriptor);
}

GPURayTracingPipeline::~GPURayTracingPipeline() {
  this->device.Reset();
  wgpuRayTracingPipelineRelease(this->instance);
}

Napi::Object GPURayTracingPipeline::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURayTracingPipeline", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURayTracingPipeline", func);
  return exports;
}
