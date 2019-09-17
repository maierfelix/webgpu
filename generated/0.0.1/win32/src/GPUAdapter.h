#ifndef __GPU_ADAPTER_H__
#define __GPU_ADAPTER_H__

#include "Base.h"

#include "GPUDevice.h"

class GPUAdapter : public Napi::ObjectWrap<GPUAdapter> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUAdapter(const Napi::CallbackInfo &info);
    ~GPUAdapter();

    // #accessors
    Napi::Value GetName(const Napi::CallbackInfo &info);
    Napi::Value GetExtensions(const Napi::CallbackInfo &info);

    Napi::Value requestDevice(const Napi::CallbackInfo &info);

    GLFWwindow* window;

    std::unique_ptr<dawn_native::Instance> nativeInstance;
    dawn_native::Adapter instance;

  private:
    GLFWwindow* GPUAdapter::createWindow(const Napi::CallbackInfo& info);
    dawn_native::Adapter GPUAdapter::createAdapter(const Napi::CallbackInfo& info);

};

#endif
