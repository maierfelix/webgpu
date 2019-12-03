#ifndef __GPU_COMPUTE_PASS_ENCODER_H__
#define __GPU_COMPUTE_PASS_ENCODER_H__

#include "Base.h"

class GPUComputePassEncoder : public Napi::ObjectWrap<GPUComputePassEncoder> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUComputePassEncoder(const Napi::CallbackInfo &info);
    ~GPUComputePassEncoder();

    // GPURenderEncoderBase BEGIN
    Napi::Value setPipeline(const Napi::CallbackInfo &info);
    Napi::Value dispatch(const Napi::CallbackInfo &info);
    Napi::Value dispatchIndirect(const Napi::CallbackInfo &info);
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

    WGPUComputePassEncoder instance;
  private:

};

#endif
