#ifndef __GPU_BUFFER_H__
#define __GPU_BUFFER_H__

#include "Base.h"

class GPUBuffer : public Napi::ObjectWrap<GPUBuffer> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPUBuffer(const Napi::CallbackInfo &info);
    ~GPUBuffer();

    Napi::Value setSubData(const Napi::CallbackInfo &info);

    Napi::Value mapReadAsync(const Napi::CallbackInfo &info);
    Napi::Value mapWriteAsync(const Napi::CallbackInfo &info);

    Napi::Value unmap(const Napi::CallbackInfo &info);
    Napi::Value destroy(const Napi::CallbackInfo &info);

    Napi::ObjectReference device;

    WGPUBuffer instance;

  private:
    // ArrayBuffers created and returned in the mapping process get linked
    // to this GPUBuffer - we keep track of them, since we have to detach them
    // after this GPUBuffer got unmapped or destroyed
    Napi::ObjectReference mappingArrayBuffers;

    void DestroyMappingArrayBuffers();
};

#endif
