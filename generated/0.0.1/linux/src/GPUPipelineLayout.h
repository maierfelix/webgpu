#ifndef __GPU_PIPELINE_LAYOUT_H__
#define __GPU_PIPELINE_LAYOUT_H__

#include "Base.h"

class GPUPipelineLayout : public Napi::ObjectWrap<GPUPipelineLayout> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUPipelineLayout(const Napi::CallbackInfo &info);
    ~GPUPipelineLayout();

    Napi::ObjectReference device;

    WGPUPipelineLayout instance;
  private:

};

#endif
