#ifndef __GPU_RAY_TRACING_ACCELERATION_CONTAINER_H__
#define __GPU_RAY_TRACING_ACCELERATION_CONTAINER_H__

#include "Base.h"

class GPURayTracingAccelerationContainer : public Napi::ObjectWrap<GPURayTracingAccelerationContainer> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURayTracingAccelerationContainer(const Napi::CallbackInfo &info);
    ~GPURayTracingAccelerationContainer();

    Napi::Value destroy(const Napi::CallbackInfo &info);

    Napi::ObjectReference device;

    WGPURayTracingAccelerationContainer instance;

  private:

};

#endif
