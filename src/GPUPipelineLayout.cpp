#include "GPUPipelineLayout.h"
#include "GPUDevice.h"
#include "GPUBindGroupLayout.h"

#include <vector>

Napi::FunctionReference GPUPipelineLayout::constructor;

GPUPipelineLayout::GPUPipelineLayout(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUPipelineLayout>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  DawnDevice backendDevice = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value())->backendDevice;

  DawnPipelineLayoutDescriptor descriptor;
  descriptor.nextInChain = nullptr;

  {
    Napi::Object obj = info[1].As<Napi::Object>();
    Napi::Array array = obj.Get("bindGroupLayouts").As<Napi::Array>();
    uint32_t length = array.Length();
    std::vector<DawnBindGroupLayout> bindGroupLayouts;
    for (unsigned int ii = 0; ii < length; ++ii) {
      Napi::Value item = array.Get(ii);
      Napi::Object obj = item.As<Napi::Object>();
      DawnBindGroupLayout bindGroupLayout = Napi::ObjectWrap<GPUBindGroupLayout>::Unwrap(obj)->bindGroupLayout;
      bindGroupLayouts.push_back(bindGroupLayout);
    };
    descriptor.bindGroupLayoutCount = length;
    descriptor.bindGroupLayouts = bindGroupLayouts.data();
  }

  this->pipelineLayout = dawnDeviceCreatePipelineLayout(backendDevice, &descriptor);
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
