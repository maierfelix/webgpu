#include "GPUCommandBuffer.h"

Napi::FunctionReference GPUCommandBuffer::constructor;

GPUCommandBuffer::GPUCommandBuffer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUCommandBuffer>(info) {

}

GPUCommandBuffer::~GPUCommandBuffer() {
  wgpuCommandBufferRelease(this->instance);
}

Napi::Object GPUCommandBuffer::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUCommandBuffer", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUCommandBuffer", func);
  return exports;
}
