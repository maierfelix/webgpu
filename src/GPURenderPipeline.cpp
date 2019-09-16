#include "GPURenderPipeline.h"
#include "GPUDevice.h"

#include <vector>

#include <shaderc/shaderc.hpp>

Napi::FunctionReference GPURenderPipeline::constructor;

GPURenderPipeline::GPURenderPipeline(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURenderPipeline>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* uwDevice = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  DawnDevice backendDevice = uwDevice->backendDevice;

  DawnRenderPipelineDescriptor descriptor;
  descriptor.nextInChain = nullptr;

  {
    Napi::Object obj = info[1].As<Napi::Object>();
    
  }

  this->renderPipeline = dawnDeviceCreateRenderPipeline(backendDevice, &descriptor);

}

GPURenderPipeline::~GPURenderPipeline() {
  // destructor
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
