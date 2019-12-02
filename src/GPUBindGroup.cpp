#include "GPUBindGroup.h"
#include "GPUDevice.h"
#include "GPUBindGroupLayout.h"
#include "GPUBuffer.h"
#include "GPUSampler.h"
#include "GPUTextureView.h"

#include "DescriptorDecoder.h"

#include <vector>

Napi::FunctionReference GPUBindGroup::constructor;

GPUBindGroup::GPUBindGroup(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUBindGroup>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPUBindGroupDescriptor(device, info[1].As<Napi::Value>());

  this->instance = dawnDeviceCreateBindGroup(device->instance, &descriptor);
}

GPUBindGroup::~GPUBindGroup() {
  // destructor
}

Napi::Object GPUBindGroup::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUBindGroup", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUBindGroup", func);
  return exports;
}
