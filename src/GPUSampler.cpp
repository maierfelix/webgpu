#include "GPUSampler.h"
#include "GPUDevice.h"

Napi::FunctionReference GPUSampler::constructor;

GPUSampler::GPUSampler(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUSampler>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  DawnDevice backendDevice = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value())->backendDevice;

  DawnSamplerDescriptor descriptor;
  descriptor.nextInChain = nullptr;
  descriptor.addressModeU = DAWN_ADDRESS_MODE_CLAMP_TO_EDGE;
  descriptor.addressModeV = DAWN_ADDRESS_MODE_CLAMP_TO_EDGE;
  descriptor.addressModeW = DAWN_ADDRESS_MODE_CLAMP_TO_EDGE;
  descriptor.magFilter = DAWN_FILTER_MODE_NEAREST;
  descriptor.minFilter = DAWN_FILTER_MODE_NEAREST;
  descriptor.mipmapFilter = DAWN_FILTER_MODE_NEAREST;
  descriptor.lodMinClamp = static_cast<float>(0);
  descriptor.lodMaxClamp = static_cast<float>(0xffffffff);
  descriptor.compare = DAWN_COMPARE_FUNCTION_NEVER;
  if (info[1].IsObject()) {
    Napi::Object obj = info[1].As<Napi::Object>();
    // addressModeU
    if (obj.Has("addressModeU")) {
      // TODO: input is a string
      descriptor.addressModeU = static_cast<DawnAddressMode>(obj.Get("addressModeU").As<Napi::Number>().Uint32Value());
    }
    // addressModeV
    if (obj.Has("addressModeV")) {
      // TODO: input is a string
      descriptor.addressModeV = static_cast<DawnAddressMode>(obj.Get("addressModeV").As<Napi::Number>().Uint32Value());
    }
    // addressModeW
    if (obj.Has("addressModeW")) {
      // TODO: input is a string
      descriptor.addressModeW = static_cast<DawnAddressMode>(obj.Get("addressModeW").As<Napi::Number>().Uint32Value());
    }
    // magFilter
    if (obj.Has("magFilter")) {
      // TODO: input is a string
      descriptor.magFilter = static_cast<DawnFilterMode>(obj.Get("magFilter").As<Napi::Number>().Uint32Value());
    }
    // minFilter
    if (obj.Has("minFilter")) {
      // TODO: input is a string
      descriptor.minFilter = static_cast<DawnFilterMode>(obj.Get("minFilter").As<Napi::Number>().Uint32Value());
    }
    // mipmapFilter
    if (obj.Has("mipmapFilter")) {
      // TODO: input is a string
      descriptor.mipmapFilter = static_cast<DawnFilterMode>(obj.Get("mipmapFilter").As<Napi::Number>().Uint32Value());
    }
    // lodMinClamp
    if (obj.Has("lodMinClamp")) {
      descriptor.lodMinClamp = obj.Get("lodMinClamp").As<Napi::Number>().FloatValue();
    }
    // lodMaxClamp
    if (obj.Has("lodMaxClamp")) {
      descriptor.lodMaxClamp = obj.Get("lodMaxClamp").As<Napi::Number>().FloatValue();
    }
    // compare
    if (obj.Has("compare")) {
      // TODO: input is a string
      descriptor.compare = static_cast<DawnCompareFunction>(obj.Get("compare").As<Napi::Number>().Uint32Value());
    }
  }

  this->sampler = dawnDeviceCreateSampler(backendDevice, &descriptor);
}

GPUSampler::~GPUSampler() {
  // destructor
}

Napi::Object GPUSampler::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUSampler", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUSampler", func);
  return exports;
}
