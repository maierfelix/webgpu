#include "GPUCanvasContext.h"
#include "GPUDevice.h"
#include "GPUAdapter.h"
#include "GPUSwapChain.h"
#include "BackendBinding.h"
#include "WebGPUWindow.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUCanvasContext::constructor;

GPUCanvasContext::GPUCanvasContext(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUCanvasContext>(info) {
  this->window.Reset(info[0].As<Napi::Object>(), 1);
}

GPUCanvasContext::~GPUCanvasContext() {
  // destructor
}

Napi::Value GPUCanvasContext::configureSwapChain(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object swapchain = GPUSwapChain::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  return swapchain;
}

Napi::Value GPUCanvasContext::getSwapChainPreferredFormat(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(info[0].As<Napi::Object>());
  GPUAdapter* adapter = Napi::ObjectWrap<GPUAdapter>::Unwrap(device->adapter.Value());
  WebGPUWindow* window = Napi::ObjectWrap<WebGPUWindow>::Unwrap(adapter->window.Value());

  if (window->preferredSwapChainFormat == WGPUTextureFormat_Undefined) {
    WGPUSwapChainDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    // returns always the same address, so we dont have to release this temp swapchain?
    descriptor.implementation = device->binding->GetSwapChainImplementation();
    WGPUSwapChain instance = wgpuDeviceCreateSwapChain(device->instance, &descriptor);
    glfwPollEvents();
    window->preferredSwapChainFormat = device->binding->GetPreferredSwapChainTextureFormat();
  }

  std::string textureFormat = DescriptorDecoder::GPUTextureFormat(
    static_cast<uint32_t>(window->preferredSwapChainFormat)
  );
  deferred.Resolve(Napi::String::New(env, textureFormat));

  return deferred.Promise();
}

Napi::Object GPUCanvasContext::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUCanvasContext", {
    InstanceMethod(
      "configureSwapChain",
      &GPUCanvasContext::configureSwapChain,
      napi_enumerable
    ),
    InstanceMethod(
      "getSwapChainPreferredFormat",
      &GPUCanvasContext::getSwapChainPreferredFormat,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUCanvasContext", func);
  return exports;
}
