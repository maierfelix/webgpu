#ifndef __GPU_DEVICE_H__
#define __GPU_DEVICE_H__

#include "Base.h"

#include "BackendBinding.h"

class GPUDevice : public Napi::ObjectWrap<GPUDevice> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUDevice(const Napi::CallbackInfo &info);
    ~GPUDevice();

    // #accessors
    Napi::Value GetExtensions(const Napi::CallbackInfo &info);
    Napi::Value GetLimits(const Napi::CallbackInfo &info);
    Napi::Value GetAdapter(const Napi::CallbackInfo &info);
    void SetOnErrorCallback(const Napi::CallbackInfo& info, const Napi::Value& value);

    Napi::Value tick(const Napi::CallbackInfo &info);
    Napi::Value getQueue(const Napi::CallbackInfo &info);
    Napi::Value createBuffer(const Napi::CallbackInfo &info);
    Napi::Value createBufferMapped(const Napi::CallbackInfo &info);
    Napi::Value createBufferMappedAsync(const Napi::CallbackInfo &info);
    Napi::Value createTexture(const Napi::CallbackInfo &info);
    Napi::Value createSampler(const Napi::CallbackInfo &info);
    Napi::Value createBindGroupLayout(const Napi::CallbackInfo &info);
    Napi::Value createPipelineLayout(const Napi::CallbackInfo &info);
    Napi::Value createBindGroup(const Napi::CallbackInfo &info);
    Napi::Value createShaderModule(const Napi::CallbackInfo &info);
    Napi::Value createComputePipeline(const Napi::CallbackInfo &info);
    Napi::Value createRenderPipeline(const Napi::CallbackInfo &info);
    Napi::Value createCommandEncoder(const Napi::CallbackInfo &info);
    Napi::Value createRenderBundleEncoder(const Napi::CallbackInfo &info);

    void throwCallbackError(const Napi::Value& type, const Napi::Value& msg);

    Napi::ObjectReference extensions;
    Napi::ObjectReference limits;
    Napi::ObjectReference adapter;

    Napi::ObjectReference mainQueue;

    Napi::FunctionReference onErrorCallback;

    dawn_native::Adapter _adapter;
    BackendBinding* binding;

    WGPUDevice instance;
  private:
    Napi::Object GPUDevice::createQueue(const Napi::CallbackInfo& info);
    BackendBinding* GPUDevice::createBinding(const Napi::CallbackInfo& info, WGPUDevice device);

};

#endif
