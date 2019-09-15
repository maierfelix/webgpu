#ifndef __GPU_TEXTURE_H__
#define __GPU_TEXTURE_H__

#include "Base.h"

class GPUTexture : public Napi::ObjectWrap<GPUTexture> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUTexture(const Napi::CallbackInfo &info);
    ~GPUTexture();

    Napi::Value createView(const Napi::CallbackInfo &info);
    Napi::Value destroy(const Napi::CallbackInfo &info);

    Napi::ObjectReference device;

    DawnTexture texture;
  private:

};

#endif
