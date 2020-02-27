#include "GPUTextureView.h"
#include "GPUTexture.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUTextureView::constructor;

GPUTextureView::GPUTextureView(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUTextureView>(info) {
  Napi::Env env = info.Env();

  // constructor called internally:
  // prevents this constructor to create a new texture,
  // since the texture is expected to be created externally
  if (info[2].IsBoolean() && info[2].As<Napi::Boolean>().Value() == true) {
    return;
  }

  this->texture.Reset(info[0].As<Napi::Object>(), 1);
  GPUTexture* texture = Napi::ObjectWrap<GPUTexture>::Unwrap(this->texture.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(texture->device.Value());

  auto descriptor = DescriptorDecoder::GPUTextureViewDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuTextureCreateView(texture->instance, &descriptor);
}

GPUTextureView::~GPUTextureView() {
  this->texture.Reset();
  wgpuTextureViewRelease(this->instance);
}

Napi::Object GPUTextureView::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUTextureView", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUTextureView", func);
  return exports;
}
