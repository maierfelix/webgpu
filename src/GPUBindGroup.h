#ifndef __GPU_BIND_GROUP_H__
#define __GPU_BIND_GROUP_H__

#include "Base.h"

class GPUBindGroup  : public Napi::ObjectWrap<GPUBindGroup > {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUBindGroup(const Napi::CallbackInfo &info);
    ~GPUBindGroup();

    Napi::ObjectReference device;

    DawnBindGroup instance;
  private:

};

#endif
