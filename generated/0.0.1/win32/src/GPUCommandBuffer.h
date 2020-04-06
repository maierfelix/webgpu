#ifndef __GPU_COMMAND_BUFFER_H__
#define __GPU_COMMAND_BUFFER_H__

#include "Base.h"

class GPUCommandBuffer : public Napi::ObjectWrap<GPUCommandBuffer> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUCommandBuffer(const Napi::CallbackInfo &info);
    ~GPUCommandBuffer();

    WGPUCommandBuffer instance;
  private:

};

#endif
