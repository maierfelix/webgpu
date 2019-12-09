#ifndef __GPU_RAY_TRACING_ACCELERATION_INSTANCE_H__
#define __GPU_RAY_TRACING_ACCELERATION_INSTANCE_H__

#include "Base.h"

class GPURayTracingAccelerationInstance : public Napi::ObjectWrap<GPURayTracingAccelerationInstance> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURayTracingAccelerationInstance(const Napi::CallbackInfo &info);
    ~GPURayTracingAccelerationInstance();

    Napi::ObjectReference device;

    WGPURayTracingAccelerationInstance instance;

  private:

};

#endif
