#ifndef __GPU_H__
#define __GPU_H__

#include "Base.h"

class GPU : public Napi::ObjectWrap<GPU> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    static Napi::Value requestAdapter(const Napi::CallbackInfo &info);
    static Napi::Value getContext(const Napi::CallbackInfo &info);

    GPU(const Napi::CallbackInfo &info);
    ~GPU();

};

#endif
