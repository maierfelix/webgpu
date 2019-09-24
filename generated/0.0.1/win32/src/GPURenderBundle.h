#ifndef __GPU_RENDER_BUNDLE_H__
#define __GPU_RENDER_BUNDLE_H__

#include "Base.h"

class GPURenderBundle : public Napi::ObjectWrap<GPURenderBundle> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURenderBundle(const Napi::CallbackInfo &info);
    ~GPURenderBundle();

    DawnRenderBundle instance;
  private:

};

#endif
