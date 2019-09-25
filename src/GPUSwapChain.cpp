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

  DawnTextureFormat format = static_cast<DawnTextureFormat>(
    DescriptorDecoder::GPUTextureFormat(args.Get("format").As<Napi::String>().Utf8Value())
  );

  DawnTextureUsage usage = DAWN_TEXTURE_USAGE_OUTPUT_ATTACHMENT;
  if (args.Has("usage")) {
    usage = static_cast<DawnTextureUsage>(
      args.Get("usage").As<Napi::Number>().Uint32Value()
    );
  }

  this->device.Reset(args.Get("device").As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  BackendBinding* binding = device->binding;

  DawnSwapChainDescriptor descriptor;
  descriptor.nextInChain = nullptr;
  descriptor.implementation = binding->GetSwapChainImplementation();

  this->instance = dawnDeviceCreateSwapChain(device->instance, &descriptor);

  dawnSwapChainConfigure(this->instance, format, usage, window->width, window->height);
}

GPUSwapChain::~GPUSwapChain() {
  // destructor
}

Napi::Value GPUSwapChain::getCurrentTexture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  DawnTexture nextTexture = dawnSwapChainGetNextTexture(this->instance);

  std::vector<napi_value> args = {
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>(),
    Napi::Boolean::New(env, true)
  };
  Napi::Object texture = GPUTexture::constructor.New(args);

  GPUTexture* uwTexture = Napi::ObjectWrap<GPUTexture>::Unwrap(texture);
  uwTexture->instance = nextTexture;

  return texture;
}

Napi::Value GPUSwapChain::present(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUTexture* texture = Napi::ObjectWrap<GPUTexture>::Unwrap(info[0].As<Napi::Object>());

  dawnSwapChainPresent(this->instance, texture->instance);

  return env.Undefined();
}

Napi::Object GPUSwapChain::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUSwapChain", {
    InstanceMethod(
      "getCurrentTexture",
      &GPUSwapChain::getCurrentTexture,
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
