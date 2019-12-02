#include "GPUPipelineLayout.h"
#include "GPUDevice.h"
#include "GPUBindGroupLayout.h"

#include "DescriptorDecoder.h"

#include <vector>

Napi::FunctionReference GPUPipelineLayout::constructor;

GPUPipelineLayout::GPUPipelineLayout(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUPipelineLayout>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPUPipelineLayoutDescriptor(device, info[1].As<Napi::Value>());

  this->instance = dawnDeviceCreatePipelineLayout(device->instance, &descriptor);
}

GPUPipelineLayout::~GPUPipelineLayout() {
  // destructor
}

Napi::Object GPUPipelineLayout::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUPipelineLayout", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUPipelineLayout", func);
  return exports;
}
