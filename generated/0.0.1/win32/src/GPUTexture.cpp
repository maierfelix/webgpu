#include "GPUTexture.h"
#include "GPUDevice.h"
#include "GPUTextureView.h"

Napi::FunctionReference GPUTexture::constructor;

GPUTexture::GPUTexture(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUTexture>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  DawnDevice backendDevice = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value())->backendDevice;

  DawnTextureDescriptor descriptor;
  descriptor.nextInChain = nullptr;
  descriptor.arrayLayerCount = 1;
  descriptor.mipLevelCount = 1;
  descriptor.sampleCount = 1;
  descriptor.dimension = DAWN_TEXTURE_DIMENSION_2D;
  {
    Napi::Object obj = info[1].As<Napi::Object>();
    // size
    {
      Napi::Object size = obj.Get("size").As<Napi::Object>();
      descriptor.size.width = size.Get("width").As<Napi::Number>().Uint32Value();
      descriptor.size.height = size.Get("height").As<Napi::Number>().Uint32Value();
      descriptor.size.depth = size.Get("depth").As<Napi::Number>().Uint32Value();
    }
    // arrayLayerCount
    if (obj.Has("arrayLayerCount")) {
      descriptor.arrayLayerCount = obj.Get("arrayLayerCount").As<Napi::Number>().Uint32Value();
    }
    // mipLevelCount
    if (obj.Has("mipLevelCount")) {
      descriptor.mipLevelCount = obj.Get("mipLevelCount").As<Napi::Number>().Uint32Value();
    }
    // sampleCount
    if (obj.Has("sampleCount")) {
      descriptor.sampleCount = obj.Get("sampleCount").As<Napi::Number>().Uint32Value();
    }
    // dimension
    if (obj.Has("dimension")) {
      // TODO: input is a string
      descriptor.dimension = DAWN_TEXTURE_DIMENSION_2D;
    }
    // format
    {
      // TODO: input is a string
      //descriptor.format = obj.Get("format").As<Napi::Number>().Uint32Value();
      descriptor.format = DAWN_TEXTURE_FORMAT_RGBA8_UNORM;
    }
    // usage
    {
      descriptor.usage = static_cast<DawnTextureUsage>(obj.Get("usage").As<Napi::Number>().Uint32Value());
    }
  }

  this->texture = dawnDeviceCreateTexture(backendDevice, &descriptor);
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
