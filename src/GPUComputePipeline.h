#ifndef __GPU_COMPUTE_PIPELINE_H__
#define __GPU_COMPUTE_PIPELINE_H__

#include "Base.h"

class GPUComputePipeline : public Napi::ObjectWrap<GPUComputePipeline> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUComputePipeline(const Napi::CallbackInfo &info);
    ~GPUComputePipeline();

    Napi::ObjectReference device;

    DawnComputePipeline instance;
  private:

};

#endif
