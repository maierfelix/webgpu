#ifndef __GPU_BUFFER_H__
#define __GPU_BUFFER_H__

#include "Base.h"

class GPUBuffer : public Napi::ObjectWrap<GPUBuffer> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUBuffer(const Napi::CallbackInfo &info);
    ~GPUBuffer();

    Napi::Value mapReadAsync(const Napi::CallbackInfo &info);
    Napi::Value mapWriteAsync(const Napi::CallbackInfo &info);

    Napi::Value unmap(const Napi::CallbackInfo &info);
    Napi::Value destroy(const Napi::CallbackInfo &info);

    Napi::ObjectReference device;

    DawnBuffer instance;

  private:

};

#endif
