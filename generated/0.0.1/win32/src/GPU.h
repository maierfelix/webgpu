#ifndef __GPU_H__
#define __GPU_H__

#include "Base.h"

#include "GPUAdapter.h"

class GPU : public Napi::ObjectWrap<GPU> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPU(const Napi::CallbackInfo &info);
    ~GPU();

};

#endif
