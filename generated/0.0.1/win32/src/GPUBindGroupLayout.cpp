#include "GPUBindGroupLayout.h"
#include "GPUDevice.h"

#include <vector>

Napi::FunctionReference GPUBindGroupLayout::constructor;

GPUBindGroupLayout::GPUBindGroupLayout(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUBindGroupLayout>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  DawnDevice backendDevice = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value())->instance;

  DawnBindGroupLayoutDescriptor descriptor;
  descriptor.nextInChain = nullptr;

  {
    Napi::Object obj = info[1].As<Napi::Object>();
    Napi::Array array = obj.Get("bindings").As<Napi::Array>();
    uint32_t length = array.Length();
    std::vector<DawnBindGroupLayoutBinding> bindGroupLayoutBindings;
    for (unsigned int ii = 0; ii < length; ++ii) {
      Napi::Value item = array.Get(ii);
      Napi::Object obj = item.As<Napi::Object>();
      DawnBindGroupLayoutBinding layoutBinding;
      {
        layoutBinding.binding = obj.Get("binding").As<Napi::Number>().Uint32Value();
        layoutBinding.visibility = static_cast<DawnShaderStage>(obj.Get("visibility").As<Napi::Number>().Uint32Value());
        layoutBinding.type = static_cast<DawnBindingType>(obj.Get("type").As<Napi::Number>().Uint32Value());
        layoutBinding.dynamic = obj.Has("dynamic") ? obj.Get("dynamic").As<Napi::Boolean>().Value() : false;
        layoutBinding.multisampled = obj.Has("multisampled") ? obj.Get("multisampled").As<Napi::Boolean>().Value() : false;
        layoutBinding.textureDimension = obj.Has("textureDimension") ? static_cast<DawnTextureViewDimension>(obj.Get("textureDimension").As<Napi::Number>().Uint32Value()) : DAWN_TEXTURE_VIEW_DIMENSION_2D;
        layoutBinding.textureComponentType = obj.Has("textureComponentType") ? static_cast<DawnTextureComponentType>(obj.Get("textureComponentType").As<Napi::Number>().Uint32Value()) : DAWN_TEXTURE_COMPONENT_TYPE_FLOAT;
      }
      bindGroupLayoutBindings.push_back(layoutBinding);
    };
    descriptor.bindingCount = length;
    descriptor.bindings = bindGroupLayoutBindings.data();
  }

  this->instance = dawnDeviceCreateBindGroupLayout(backendDevice, &descriptor);
}

GPUBindGroupLayout::~GPUBindGroupLayout() {
  // destructor
}

Napi::Object GPUBindGroupLayout::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUBindGroupLayout", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUBindGroupLayout", func);
  return exports;
}
