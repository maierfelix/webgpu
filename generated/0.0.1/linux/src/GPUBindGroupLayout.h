#ifndef __GPU_BIND_GROUP_LAYOUT_H__
#define __GPU_BIND_GROUP_LAYOUT_H__

#include "Base.h"

class GPUBindGroupLayout : public Napi::ObjectWrap<GPUBindGroupLayout> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUBindGroupLayout(const Napi::CallbackInfo &info);
    ~GPUBindGroupLayout();

    Napi::ObjectReference device;

    WGPUBindGroupLayout instance;
  private:

};

#endif
