#ifndef __GPU_RENDER_PASS_ENCODER_H__
#define __GPU_RENDER_PASS_ENCODER_H__

#include "Base.h"

// no inheritance in NAPI
// GPURenderEncoderBase : GPUProgrammablePassEncoder
// GPURenderPassEncoder : GPURenderEncoderBase

class GPURenderPassEncoder : public Napi::ObjectWrap<GPURenderPassEncoder> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURenderPassEncoder(const Napi::CallbackInfo &info);
    ~GPURenderPassEncoder();

    // GPURenderEncoderBase BEGIN
    Napi::Value setPipeline(const Napi::CallbackInfo &info);
    Napi::Value setIndexBuffer(const Napi::CallbackInfo &info);
    Napi::Value setVertexBuffer(const Napi::CallbackInfo &info);
    Napi::Value draw(const Napi::CallbackInfo &info);
    Napi::Value drawIndexed(const Napi::CallbackInfo &info);
    Napi::Value drawIndirect(const Napi::CallbackInfo &info);
    Napi::Value drawIndexedIndirect(const Napi::CallbackInfo &info);
    // GPURenderEncoderBase END

    // GPURenderPassEncoder BEGIN
    Napi::Value setViewport(const Napi::CallbackInfo &info);
    Napi::Value setScissorRect(const Napi::CallbackInfo &info);
    Napi::Value setBlendColor(const Napi::CallbackInfo &info);
    Napi::Value setStencilReference(const Napi::CallbackInfo &info);

    Napi::Value executeBundles(const Napi::CallbackInfo &info);

    Napi::Value endPass(const Napi::CallbackInfo &info);
    // GPURenderPassEncoder END

    // GPUProgrammablePassEncoder BEGIN
    Napi::Value setBindGroup(const Napi::CallbackInfo &info);
    Napi::Value pushDebugGroup(const Napi::CallbackInfo &info);
    Napi::Value popDebugGroup(const Napi::CallbackInfo &info);
    Napi::Value insertDebugMarker(const Napi::CallbackInfo &info);
    // GPUProgrammablePassEncoder END

    Napi::ObjectReference device;
    Napi::ObjectReference commandEncoder;

    WGPURenderPassEncoder instance;
  private:

};

#endif
