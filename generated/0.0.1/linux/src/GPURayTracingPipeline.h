#ifndef __GPU_RAY_TRACING_PIPELINE_H__
#define __GPU_RAY_TRACING_PIPELINE_H__

#include "Base.h"

class GPURayTracingPipeline : public Napi::ObjectWrap<GPURayTracingPipeline> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURayTracingPipeline(const Napi::CallbackInfo &info);
    ~GPURayTracingPipeline();

    Napi::ObjectReference device;

    WGPURayTracingPipeline instance;
  private:

};

#endif
