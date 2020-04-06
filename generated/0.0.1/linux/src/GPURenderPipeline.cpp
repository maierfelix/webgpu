#include "GPURenderPipeline.h"
#include "GPUDevice.h"
#include "GPUShaderModule.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPURenderPipeline::constructor;

GPURenderPipeline::GPURenderPipeline(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURenderPipeline>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPURenderPipelineDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuDeviceCreateRenderPipeline(device->instance, &descriptor);
}

GPURenderPipeline::~GPURenderPipeline() {
  this->device.Reset();
  wgpuRenderPipelineRelease(this->instance);
}

Napi::Object GPURenderPipeline::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURenderPipeline", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURenderPipeline", func);
  return exports;
}
