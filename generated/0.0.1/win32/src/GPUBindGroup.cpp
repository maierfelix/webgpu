#include "GPUBindGroup.h"
#include "GPUDevice.h"
#include "GPUBindGroupLayout.h"
#include "GPUBuffer.h"
#include "GPUSampler.h"
#include "GPUTextureView.h"

#include <vector>

Napi::FunctionReference GPUBindGroup::constructor;

GPUBindGroup::GPUBindGroup(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUBindGroup>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  DawnDevice backendDevice = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value())->backendDevice;

  DawnBindGroupDescriptor descriptor;
  descriptor.nextInChain = nullptr;

  {
    Napi::Object obj = info[1].As<Napi::Object>();
    // layout
    {
      Napi::Object layout = obj.Get("layout").As<Napi::Object>();
      descriptor.layout = Napi::ObjectWrap<GPUBindGroupLayout>::Unwrap(layout)->bindGroupLayout;
    }
    // bindings
    {
      Napi::Array array = obj.Get("bindings").As<Napi::Array>();
      uint32_t length = array.Length();
      std::vector<DawnBindGroupBinding> bindGroupBindings;
      for (unsigned int ii = 0; ii < length; ++ii) {
        Napi::Value item = array.Get(ii);
        Napi::Object obj = item.As<Napi::Object>();
        DawnBindGroupBinding bindGroupBinding;
        {
          bindGroupBinding.binding = obj.Get("binding").As<Napi::Number>().Uint32Value();
          Napi::Object resource = obj.Get("resource").As<Napi::Object>();
          // GPUBuffer
          if (resource.InstanceOf(GPUBuffer::constructor.Value())) {
            Napi::Object bufferBinding = obj.As<Napi::Object>();
            GPUBuffer* buffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(
              bufferBinding.Get("buffer").As<Napi::Object>()
            );
            bindGroupBinding.buffer = buffer->buffer;
            if (bufferBinding.Has("offset")) {
              bool lossless;
              bindGroupBinding.offset = bufferBinding.Get("offset").As<Napi::BigInt>().Uint64Value(&lossless);
            }
            if (bufferBinding.Has("size")) {
              bool lossless;
              bindGroupBinding.size = bufferBinding.Get("size").As<Napi::BigInt>().Uint64Value(&lossless);
            }
          }
          // GPUSampler
          else if (resource.InstanceOf(GPUSampler::constructor.Value())) {
            bindGroupBinding.sampler = Napi::ObjectWrap<GPUSampler>::Unwrap(resource)->sampler;
          }
          // GPUTextureView
          else if (resource.InstanceOf(GPUTextureView::constructor.Value())) {
            bindGroupBinding.textureView = Napi::ObjectWrap<GPUTextureView>::Unwrap(resource)->textureView;
          }
        }
        bindGroupBindings.push_back(bindGroupBinding);
      };
      descriptor.bindingCount = length;
      descriptor.bindings = bindGroupBindings.data();
    }
  }

  this->bindGroup = dawnDeviceCreateBindGroup(backendDevice, &descriptor);
}

GPUBindGroup::~GPUBindGroup() {
  // destructor
}

Napi::Object GPUBindGroup::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUBindGroup", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUBindGroup", func);
  return exports;
}
