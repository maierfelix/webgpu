#include "GPUSwapChain.h"
#include "GPUDevice.h"
#include "GPUTexture.h"
#include "BackendBinding.h"
#include "GPUCanvasContext.h"
#include "WebGPUWindow.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUSwapChain::constructor;

GPUSwapChain::GPUSwapChain(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUSwapChain>(info) {
  Napi::Env env = info.Env();

  this->context.Reset(info[0].As<Napi::Object>(), 1);
  GPUCanvasContext* context = Napi::ObjectWrap<GPUCanvasContext>::Unwrap(this->context.Value());
  WebGPUWindow* window = Napi::ObjectWrap<WebGPUWindow>::Unwrap(context->window.Value());

  Napi::Object args = info[1].As<Napi::Object>();

  this->device.Reset(args.Get("device").As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  // create
  WGPUSwapChainDescriptor descriptor;
  descriptor.nextInChain = nullptr;
  descriptor.implementation = device->binding->GetSwapChainImplementation();

  this->instance = wgpuDeviceCreateSwapChain(device->instance, nullptr, &descriptor);

  // configurate
  WGPUTextureFormat format = static_cast<WGPUTextureFormat>(
    DescriptorDecoder::GPUTextureFormat(args.Get("format").As<Napi::String>().Utf8Value())
  );

  WGPUTextureUsage usage = WGPUTextureUsage_OutputAttachment;
  if (args.Has("usage")) {
    usage = static_cast<WGPUTextureUsage>(args.Get("usage").As<Napi::Number>().Uint32Value());
  }

  wgpuSwapChainConfigure(this->instance, format, usage, window->width, window->height);

  this->format = format;
  this->usage = usage;

  window->swapChain = this;
}

GPUSwapChain::~GPUSwapChain() {
  this->device.Reset();
  this->context.Reset();
  wgpuSwapChainRelease(this->instance);
}

Napi::Value GPUSwapChain::getCurrentTextureView(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  WGPUTextureView nextTextureView = wgpuSwapChainGetCurrentTextureView(this->instance);

  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>(),
    Napi::Boolean::New(env, true)
  };
  Napi::Object textureView = GPUTextureView::constructor.New(args);

  GPUTextureView* uwTexture = Napi::ObjectWrap<GPUTextureView>::Unwrap(textureView);
  uwTexture->instance = nextTextureView;

  return textureView;
}

Napi::Value GPUSwapChain::present(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuSwapChainPresent(this->instance);

  return env.Undefined();
}

Napi::Object GPUSwapChain::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUSwapChain", {
    InstanceMethod(
      "getCurrentTextureView",
      &GPUSwapChain::getCurrentTextureView,
      napi_enumerable
    ),
    InstanceMethod(
      "present",
      &GPUSwapChain::present,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUSwapChain", func);
  return exports;
}
