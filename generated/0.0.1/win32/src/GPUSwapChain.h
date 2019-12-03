#ifndef __GPU_SWAPCHAIN_H__
#define __GPU_SWAPCHAIN_H__

#include "Base.h"

class GPUSwapChain : public Napi::ObjectWrap<GPUSwapChain> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUSwapChain(const Napi::CallbackInfo &info);
    ~GPUSwapChain();

    Napi::Value getCurrentTextureView(const Napi::CallbackInfo &info);
    Napi::Value present(const Napi::CallbackInfo &info);

    Napi::ObjectReference device;
    Napi::ObjectReference context;

    WGPUSwapChain instance;

    WGPUTextureFormat format;
    WGPUTextureUsage usage;
};

#endif
