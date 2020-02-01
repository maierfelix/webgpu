#include "GPUTexture.h"
#include "GPUDevice.h"
#include "GPUTextureView.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUTexture::constructor;

GPUTexture::GPUTexture(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUTexture>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);

  // constructor called internally:
  // prevents this constructor to create a new texture,
  // since the texture is expected to be created externally
  if (info[2].IsBoolean() && info[2].As<Napi::Boolean>().Value() == true) {
    return;
  }
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPUTextureDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuDeviceCreateTexture(device->instance, &descriptor);

  this->dimension = (&descriptor)->textureDimension;
}

GPUTexture::~GPUTexture() {
  this->device.Reset();
  wgpuTextureRelease(this->instance);
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
