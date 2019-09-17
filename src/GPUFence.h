#ifndef __GPU_FENCE_H__
#define __GPU_FENCE_H__

#include "Base.h"

class GPUFence : public Napi::ObjectWrap<GPUFence> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUFence(const Napi::CallbackInfo &info);
    ~GPUFence();

    Napi::Value getCompletedValue(const Napi::CallbackInfo &info);
    Napi::Value onCompletion(const Napi::CallbackInfo &info);

    Napi::ObjectReference queue;
    Napi::ObjectReference device;

    DawnFence instance;
  private:

};

#endif
