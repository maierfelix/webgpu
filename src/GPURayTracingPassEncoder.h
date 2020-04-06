#ifndef __GPU_RAY_TRACING_PASS_ENCODER_H__
#define __GPU_RAY_TRACING_PASS_ENCODER_H__

#include "Base.h"

class GPURayTracingPassEncoder : public Napi::ObjectWrap<GPURayTracingPassEncoder> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURayTracingPassEncoder(const Napi::CallbackInfo &info);
    ~GPURayTracingPassEncoder();

    // GPURenderEncoderBase BEGIN
    Napi::Value setPipeline(const Napi::CallbackInfo &info);
    Napi::Value traceRays(const Napi::CallbackInfo &info);
    Napi::Value endPass(const Napi::CallbackInfo &info);
    // GPURenderEncoderBase END

    // GPUProgrammablePassEncoder BEGIN
    Napi::Value setBindGroup(const Napi::CallbackInfo &info);
    Napi::Value pushDebugGroup(const Napi::CallbackInfo &info);
    Napi::Value popDebugGroup(const Napi::CallbackInfo &info);
    Napi::Value insertDebugMarker(const Napi::CallbackInfo &info);
    // GPUProgrammablePassEncoder END

    Napi::ObjectReference device;
    Napi::ObjectReference commandEncoder;

    WGPURayTracingPassEncoder instance;
  private:

};

#endif
