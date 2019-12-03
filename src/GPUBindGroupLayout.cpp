#include "GPUBindGroupLayout.h"
#include "GPUDevice.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUBindGroupLayout::constructor;

GPUBindGroupLayout::GPUBindGroupLayout(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUBindGroupLayout>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPUBindGroupLayoutDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuDeviceCreateBindGroupLayout(device->instance, &descriptor);
}

GPUBindGroupLayout::~GPUBindGroupLayout() {
  // destructor
}

Napi::Object GPUBindGroupLayout::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUBindGroupLayout", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUBindGroupLayout", func);
  return exports;
}
