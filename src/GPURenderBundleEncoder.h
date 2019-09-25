#ifndef __GPU_RENDER_BUNDLE_ENCODER_H__
#define __GPU_RENDER_BUNDLE_ENCODER_H__

#include "Base.h"

class GPURenderBundleEncoder : public Napi::ObjectWrap<GPURenderBundleEncoder> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURenderBundleEncoder(const Napi::CallbackInfo &info);
    ~GPURenderBundleEncoder();

    // GPURenderEncoderBase BEGIN
    Napi::Value setPipeline(const Napi::CallbackInfo &info);
    Napi::Value setIndexBuffer(const Napi::CallbackInfo &info);
    Napi::Value setVertexBuffers(const Napi::CallbackInfo &info);
    Napi::Value draw(const Napi::CallbackInfo &info);
    Napi::Value drawIndexed(const Napi::CallbackInfo &info);
    Napi::Value drawIndirect(const Napi::CallbackInfo &info);
    Napi::Value drawIndexedIndirect(const Napi::CallbackInfo &info);
    // GPURenderEncoderBase END

    // GPURenderBundleEncoder BEGIN
    Napi::Value finish(const Napi::CallbackInfo &info);
    // GPURenderBundleEncoder END

    // GPUProgrammablePassEncoder BEGIN
    Napi::Value setBindGroup(const Napi::CallbackInfo &info);
    Napi::Value pushDebugGroup(const Napi::CallbackInfo &info);
    Napi::Value popDebugGroup(const Napi::CallbackInfo &info);
    Napi::Value insertDebugMarker(const Napi::CallbackInfo &info);
    // GPUProgrammablePassEncoder END

    Napi::ObjectReference device;
    Napi::ObjectReference commandEncoder;

    DawnRenderBundleEncoder instance;
  private:

};

#endif
