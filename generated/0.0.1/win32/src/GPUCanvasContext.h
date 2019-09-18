#ifndef __GPU_CANVAS_CONTEXT_H__
#define __GPU_CANVAS_CONTEXT_H__

#include "Base.h"

class GPUCanvasContext : public Napi::ObjectWrap<GPUCanvasContext> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUCanvasContext(const Napi::CallbackInfo &info);
    ~GPUCanvasContext();

    Napi::Value configureSwapChain(const Napi::CallbackInfo &info);
    Napi::Value getSwapChainPreferredFormat(const Napi::CallbackInfo &info);
};

#endif
