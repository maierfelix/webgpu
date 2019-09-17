#include "GPUTextureView.h"
#include "GPUTexture.h"

Napi::FunctionReference GPUTextureView::constructor;

GPUTextureView::GPUTextureView(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUTextureView>(info) {
  Napi::Env env = info.Env();

  this->texture.Reset(info[0].As<Napi::Object>(), 1);
  DawnTexture texture = Napi::ObjectWrap<GPUTexture>::Unwrap(this->texture.Value())->instance;

  DawnTextureViewDescriptor descriptor;
  descriptor.nextInChain = nullptr;
  descriptor.aspect = DAWN_TEXTURE_ASPECT_ALL;
  descriptor.baseMipLevel = 0;
  descriptor.mipLevelCount = 0;
  descriptor.baseArrayLayer = 0;
  descriptor.arrayLayerCount = 0;
  if (info[1].IsObject()) {
    Napi::Object obj = info[1].As<Napi::Object>();
    // format
    if (obj.Has("format")) {
      // TODO: input is a string
      descriptor.format = static_cast<DawnTextureFormat>(obj.Get("format").As<Napi::Number>().Uint32Value());
    }
    // dimension
    if (obj.Has("dimension")) {
      // TODO: input is a string
      descriptor.dimension = static_cast<DawnTextureViewDimension>(obj.Get("dimension").As<Napi::Number>().Uint32Value());
    }
    // aspect
    if (obj.Has("aspect")) {
      // TODO: input is a string
      descriptor.aspect = static_cast<DawnTextureAspect>(obj.Get("aspect").As<Napi::Number>().Uint32Value());
    }
    // baseMipLevel
    if (obj.Has("baseMipLevel")) {
      descriptor.baseMipLevel = obj.Get("baseMipLevel").As<Napi::Number>().Uint32Value();
    }
    // mipLevelCount
    if (obj.Has("mipLevelCount")) {
      descriptor.mipLevelCount = obj.Get("mipLevelCount").As<Napi::Number>().Uint32Value();
    }
    // baseArrayLayer
    if (obj.Has("baseArrayLayer")) {
      descriptor.baseArrayLayer = obj.Get("baseArrayLayer").As<Napi::Number>().Uint32Value();
    }
    // arrayLayerCount
    if (obj.Has("arrayLayerCount")) {
      descriptor.arrayLayerCount = obj.Get("arrayLayerCount").As<Napi::Number>().Uint32Value();
    }
  }

  this->instance = dawnTextureCreateView(texture, &descriptor);
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
