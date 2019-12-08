#ifndef __GPU_RAY_TRACING_ACCELERATION_GEOMETRY_H__
#define __GPU_RAY_TRACING_ACCELERATION_GEOMETRY_H__

#include "Base.h"

class GPURayTracingAccelerationGeometry : public Napi::ObjectWrap<GPURayTracingAccelerationGeometry> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    GPURayTracingAccelerationGeometry(const Napi::CallbackInfo &info);
    ~GPURayTracingAccelerationGeometry();

    Napi::ObjectReference device;

    WGPURayTracingAccelerationGeometry instance;

  private:

};

#endif
