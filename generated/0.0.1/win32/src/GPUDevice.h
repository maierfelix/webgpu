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

    Napi::ObjectReference extensions;
    Napi::ObjectReference limits;
    Napi::ObjectReference adapter;

    Napi::ObjectReference mainQueue;

    Napi::FunctionReference onErrorCallback;

    dawn_native::Adapter _adapter;
    BackendBinding* binding;

    dawn::Device device;
    DawnDevice backendDevice;
  private:
    DawnDevice GPUDevice::createDevice();
    Napi::Object GPUDevice::createQueue(const Napi::CallbackInfo& info);
    BackendBinding* GPUDevice::createBinding(const Napi::CallbackInfo& info, DawnDevice device);

};

#endif
