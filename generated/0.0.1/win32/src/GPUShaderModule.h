#ifndef __GPU_SHADER_MODULE_H__
#define __GPU_SHADER_MODULE_H__

#include "Base.h"

class GPUShaderModule  : public Napi::ObjectWrap<GPUShaderModule > {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUShaderModule(const Napi::CallbackInfo &info);
    ~GPUShaderModule();

    Napi::ObjectReference device;

    DawnShaderModule instance;
  private:

};

#endif
