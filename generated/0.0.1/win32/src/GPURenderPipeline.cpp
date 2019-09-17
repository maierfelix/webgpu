#include "GPURenderPipeline.h"
#include "GPUDevice.h"
#include "GPUShaderModule.h"

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
    // vertexStage
    {
      Napi::Object vertexStage = obj.Get("vertexStage").As<Napi::Object>();
      // vertexStage.module
      {
        Napi::Object module = vertexStage.Get("module").As<Napi::Object>();
        descriptor.vertexStage.module = Napi::ObjectWrap<GPUShaderModule>::Unwrap(module)->shaderModule;
      }
      // vertexStage.entryPoint
      {
        Napi::Object entryPoint = vertexStage.Get("entryPoint").As<Napi::Object>();
        descriptor.vertexStage.entryPoint = entryPoint.As<Napi::String>().Utf8Value().c_str();
      }
    }
    // fragmentStage
    if (obj.Has("fragmentStage")) {
      DawnPipelineStageDescriptor* fragmentStageDescriptor = new DawnPipelineStageDescriptor;
      Napi::Object fragmentStage = obj.Get("fragmentStage").As<Napi::Object>();
      // fragmentStage.module
      {
        Napi::Object module = fragmentStage.Get("module").As<Napi::Object>();
        fragmentStageDescriptor->module = Napi::ObjectWrap<GPUShaderModule>::Unwrap(module)->shaderModule;
      }
      // fragmentStage.entryPoint
      {
        Napi::Object entryPoint = fragmentStage.Get("entryPoint").As<Napi::Object>();
        fragmentStageDescriptor->entryPoint = getNAPIStringCopy(entryPoint);
      }
      descriptor.fragmentStage = fragmentStageDescriptor;
    }
    // primitiveTopology
    {
      descriptor.primitiveTopology = static_cast<DawnPrimitiveTopology>(obj.Get("primitiveTopology").As<Napi::Number>().Uint32Value());
    }
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
