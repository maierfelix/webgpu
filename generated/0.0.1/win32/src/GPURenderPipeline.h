#ifndef __GPU_RENDER_PIPELINE_H__
#define __GPU_RENDER_PIPELINE_H__

#include "Base.h"

class GPURenderPipeline  : public Napi::ObjectWrap<GPURenderPipeline > {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURenderPipeline(const Napi::CallbackInfo &info);
    ~GPURenderPipeline();

    Napi::ObjectReference device;

    DawnRenderPipeline instance;
  private:

};

#endif
