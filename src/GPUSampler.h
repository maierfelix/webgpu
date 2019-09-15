#ifndef __GPU_SAMPLER_H__
#define __GPU_SAMPLER_H__

#include "Base.h"

class GPUSampler : public Napi::ObjectWrap<GPUSampler> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUSampler(const Napi::CallbackInfo &info);
    ~GPUSampler();

    Napi::ObjectReference device;

    DawnSampler sampler;
  private:

};

#endif
