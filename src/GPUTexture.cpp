#include "GPUTexture.h"
#include "GPUDevice.h"
#include "GPUTextureView.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUTexture::constructor;

GPUTexture::GPUTexture(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUTexture>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  DawnTextureDescriptor descriptor = DescriptorDecoder::GPUTextureDescriptor(device, info[1].As<Napi::Value>());

  this->instance = dawnDeviceCreateTexture(device->instance, &descriptor);
}

GPUTexture::~GPUTexture() {
  // destructor
}

Napi::Value GPUTexture::createView(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>()
  };
  if (info[0].IsObject()) args.push_back(info[0].As<Napi::Value>());
  Napi::Object textureView = GPUTextureView::constructor.New(args);
  return textureView;
}

Napi::Value GPUTexture::destroy(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  return env.Undefined();
}

Napi::Object GPUTexture::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUTexture", {
    InstanceMethod(
      "createView",
      &GPUTexture::createView,
      napi_enumerable
    ),
    InstanceMethod(
      "destroy",
      &GPUTexture::destroy,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUTexture", func);
  return exports;
}
