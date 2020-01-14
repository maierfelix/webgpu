#include "GPURayTracingPassEncoder.h"
#include "GPUDevice.h"
#include "GPUCommandEncoder.h"
#include "GPURayTracingPipeline.h"
#include "GPUBindGroup.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPURayTracingPassEncoder::constructor;

GPURayTracingPassEncoder::GPURayTracingPassEncoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURayTracingPassEncoder>(info) {
  Napi::Env env = info.Env();

  this->commandEncoder.Reset(info[0].As<Napi::Object>(), 1);
  GPUCommandEncoder* commandEncoder = Napi::ObjectWrap<GPUCommandEncoder>::Unwrap(this->commandEncoder.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(commandEncoder->device.Value());

  auto descriptor = DescriptorDecoder::GPURayTracingPassDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuCommandEncoderBeginRayTracingPass(commandEncoder->instance, &descriptor);
}

GPURayTracingPassEncoder::~GPURayTracingPassEncoder() {
  this->device.Reset();
  this->commandEncoder.Reset();
  wgpuRayTracingPassEncoderRelease(this->instance);
}

Napi::Value GPURayTracingPassEncoder::setPipeline(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPURayTracingPipeline* rayTracingPipeline = Napi::ObjectWrap<GPURayTracingPipeline>::Unwrap(info[0].As<Napi::Object>());

  wgpuRayTracingPassEncoderSetPipeline(this->instance, rayTracingPipeline->instance);

  return env.Undefined();
}

Napi::Value GPURayTracingPassEncoder::traceRays(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t width = info[0].As<Napi::Number>().Uint32Value();
  uint32_t height = info[1].As<Napi::Number>().Uint32Value();
  uint32_t depth = info[2].IsNumber() ? info[2].As<Napi::Number>().Uint32Value() : 1;

  wgpuRayTracingPassEncoderTraceRays(this->instance, width, height, depth);

  return env.Undefined();
}

Napi::Value GPURayTracingPassEncoder::endPass(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuRayTracingPassEncoderEndPass(this->instance);

  return env.Undefined();
}

Napi::Value GPURayTracingPassEncoder::setBindGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t groupIndex = info[0].As<Napi::Number>().Uint32Value();

  WGPUBindGroup group = Napi::ObjectWrap<GPUBindGroup>::Unwrap(info[1].As<Napi::Object>())->instance;

  uint32_t dynamicOffsetCount = 0;
  std::vector<uint32_t> dynamicOffsets;
  if (info[2].IsArray()) {
    Napi::Array array = info[2].As<Napi::Array>();
    for (unsigned int ii = 0; ii < array.Length(); ++ii) {
      uint32_t offset = array.Get(ii).As<Napi::Number>().Uint32Value();
      dynamicOffsets.push_back(offset);
    };
    dynamicOffsetCount = array.Length();
  }

  wgpuRayTracingPassEncoderSetBindGroup(this->instance, groupIndex, group, dynamicOffsetCount, dynamicOffsets.data());

  return env.Undefined();
}

Napi::Value GPURayTracingPassEncoder::pushDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuRayTracingPassEncoderPushDebugGroup(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPURayTracingPassEncoder::popDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuRayTracingPassEncoderPopDebugGroup(this->instance);

  return env.Undefined();
}

Napi::Value GPURayTracingPassEncoder::insertDebugMarker(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuRayTracingPassEncoderInsertDebugMarker(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Object GPURayTracingPassEncoder::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURayTracingPassEncoder", {
    InstanceMethod(
      "setPipeline",
      &GPURayTracingPassEncoder::setPipeline,
      napi_enumerable
    ),
    InstanceMethod(
      "traceRays",
      &GPURayTracingPassEncoder::traceRays,
      napi_enumerable
    ),
    InstanceMethod(
      "endPass",
      &GPURayTracingPassEncoder::endPass,
      napi_enumerable
    ),
    InstanceMethod(
      "setBindGroup",
      &GPURayTracingPassEncoder::setBindGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "pushDebugGroup",
      &GPURayTracingPassEncoder::pushDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "popDebugGroup",
      &GPURayTracingPassEncoder::popDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "insertDebugMarker",
      &GPURayTracingPassEncoder::insertDebugMarker,
      napi_enumerable
    ),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURayTracingPassEncoder", func);
  return exports;
}
