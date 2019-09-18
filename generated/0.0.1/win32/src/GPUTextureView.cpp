#include "GPUTextureView.h"
#include "GPUTexture.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUTextureView::constructor;

GPUTextureView::GPUTextureView(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUTextureView>(info) {
  Napi::Env env = info.Env();

  this->texture.Reset(info[0].As<Napi::Object>(), 1);
  GPUTexture* texture = Napi::ObjectWrap<GPUTexture>::Unwrap(this->texture.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(texture->device.Value());

  DawnTextureViewDescriptor descriptor = DescriptorDecoder::GPUTextureViewDescriptor(device, info[1].As<Napi::Value>());

  this->instance = dawnTextureCreateView(texture->instance, &descriptor);
}

GPUTextureView::~GPUTextureView() {
  // destructor
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
