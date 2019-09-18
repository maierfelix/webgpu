#include "GPUSwapChain.h"
#include "GPUDevice.h"
#include "GPUTexture.h"
#include "BackendBinding.h"

Napi::FunctionReference GPUSwapChain::constructor;

GPUSwapChain::GPUSwapChain(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUSwapChain>(info) {
  Napi::Env env = info.Env();

  Napi::Object args = info[0].As<Napi::Object>();
  /*
  // seems to be ignored right now
  DawnTextureFormat textureFormat = static_cast<DawnTextureFormat>(
    args.Get("format").As<Napi::Number>().Uint32Value()
  );
  */
  /*
  // seems to be ignored right now
  DawnTextureFormat usage = static_cast<DawnTextureUsage>(
    args.Get("usage").As<Napi::Number>().Uint32Value()
  );
  */
  this->device.Reset(args.Get("device").As<Napi::Object>(), 1);

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  BackendBinding* binding = device->binding;

  DawnSwapChainDescriptor descriptor;
  descriptor.nextInChain = nullptr;
  descriptor.implementation = binding->GetSwapChainImplementation();

  this->instance = dawnDeviceCreateSwapChain(device->instance, &descriptor);
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
  Napi::Object out = GPUTexture::constructor.New(args);

  GPUTexture* texture = Napi::ObjectWrap<GPUTexture>::Unwrap(out);
  texture->instance = nextTexture;

  return out;
};

Napi::Object GPUSwapChain::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUSwapChain", {
    InstanceMethod(
      "getCurrentTexture",
      &GPUSwapChain::getCurrentTexture,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUSwapChain", func);
  return exports;
}
