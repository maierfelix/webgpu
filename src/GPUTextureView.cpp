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

  Napi::Object opts = info[1].As<Napi::Object>();

  // if dimension is unspecified, do:
  // https://gpuweb.github.io/gpuweb/#texture-view-creation
  if (!(opts.Has("dimension"))) {
    WGPUTextureDimension srcDimension = texture->dimension;
    WGPUTextureViewDimension dstDimension = WGPUTextureViewDimension_Undefined;
    switch (srcDimension) {
      case WGPUTextureDimension_1D: {
        dstDimension = WGPUTextureViewDimension_1D;
      } break;
      case WGPUTextureDimension_2D: {
        if (texture->arrayLayerCount > 1 && (&descriptor)->arrayLayerCount == 0) {
          dstDimension = WGPUTextureViewDimension_2DArray;
        } else {
          dstDimension = WGPUTextureViewDimension_2D;
        }
      } break;
      case WGPUTextureDimension_3D: {
        dstDimension = WGPUTextureViewDimension_3D;
      } break;
    };
    (&descriptor)->dimension = dstDimension;
  }

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
