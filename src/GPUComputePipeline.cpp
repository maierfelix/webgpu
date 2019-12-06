#include "GPUComputePipeline.h"
#include "GPUDevice.h"
#include "GPUShaderModule.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUComputePipeline::constructor;

GPUComputePipeline::GPUComputePipeline(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUComputePipeline>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPUComputePipelineDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuDeviceCreateComputePipeline(device->instance, &descriptor);
}

GPUComputePipeline::~GPUComputePipeline() {
  this->device.Reset();
  wgpuComputePipelineRelease(this->instance);
}

Napi::Object GPUComputePipeline::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUComputePipeline", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUComputePipeline", func);
  return exports;
}
