#include "GPUComputePassEncoder.h"
#include "GPUDevice.h"
#include "GPUCommandEncoder.h"
#include "GPUComputePipeline.h"
#include "GPUBuffer.h"
#include "GPUBindGroup.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUComputePassEncoder::constructor;

GPUComputePassEncoder::GPUComputePassEncoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUComputePassEncoder>(info) {
  Napi::Env env = info.Env();

  this->commandEncoder.Reset(info[0].As<Napi::Object>(), 1);
  GPUCommandEncoder* commandEncoder = Napi::ObjectWrap<GPUCommandEncoder>::Unwrap(this->commandEncoder.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(commandEncoder->device.Value());

  auto descriptor = DescriptorDecoder::GPUComputePassDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuCommandEncoderBeginComputePass(commandEncoder->instance, &descriptor);
}

GPUComputePassEncoder::~GPUComputePassEncoder() {
  // destructor
}

Napi::Value GPUComputePassEncoder::setPipeline(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUComputePipeline* computePipeline = Napi::ObjectWrap<GPUComputePipeline>::Unwrap(info[0].As<Napi::Object>());

  wgpuComputePassEncoderSetPipeline(this->instance, computePipeline->instance);

  return env.Undefined();
}

Napi::Value GPUComputePassEncoder::dispatch(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t x = info[0].IsNumber() ? info[0].As<Napi::Number>().Uint32Value() : 1;
  uint32_t y = info[1].IsNumber() ? info[1].As<Napi::Number>().Uint32Value() : 1;
  uint32_t z = info[2].IsNumber() ? info[2].As<Napi::Number>().Uint32Value() : 1;

  wgpuComputePassEncoderDispatch(this->instance, x, y, z);

  return env.Undefined();
}

Napi::Value GPUComputePassEncoder::dispatchIndirect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  bool lossless;

  GPUBuffer* indirectBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t indirectOffset = info[1].As<Napi::BigInt>().Uint64Value(&lossless);

  wgpuComputePassEncoderDispatchIndirect(this->instance, indirectBuffer->instance, indirectOffset);

  return env.Undefined();
}

Napi::Value GPUComputePassEncoder::endPass(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuComputePassEncoderEndPass(this->instance);

  return env.Undefined();
}

Napi::Value GPUComputePassEncoder::setBindGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t groupIndex = info[0].As<Napi::Number>().Uint32Value();

  WGPUBindGroup group = Napi::ObjectWrap<GPUBindGroup>::Unwrap(info[1].As<Napi::Object>())->instance;

  uint32_t dynamicOffsetCount;
  std::vector<uint32_t> dynamicOffsets;
  if (info[2].IsArray()) {
    Napi::Array array = info[2].As<Napi::Array>();
    for (unsigned int ii = 0; ii < array.Length(); ++ii) {
      uint32_t offset = array.Get(ii).As<Napi::Number>().Uint32Value();
      dynamicOffsets.push_back(offset);
    };
    dynamicOffsetCount = array.Length();
  }

  wgpuComputePassEncoderSetBindGroup(this->instance, groupIndex, group, dynamicOffsetCount, dynamicOffsets.data());

  return env.Undefined();
}

Napi::Value GPUComputePassEncoder::pushDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuComputePassEncoderPushDebugGroup(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPUComputePassEncoder::popDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuComputePassEncoderPopDebugGroup(this->instance);

  return env.Undefined();
}

Napi::Value GPUComputePassEncoder::insertDebugMarker(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuComputePassEncoderInsertDebugMarker(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Object GPUComputePassEncoder::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUComputePassEncoder", {
    InstanceMethod(
      "setPipeline",
      &GPUComputePassEncoder::setPipeline,
      napi_enumerable
    ),
    InstanceMethod(
      "dispatch",
      &GPUComputePassEncoder::dispatch,
      napi_enumerable
    ),
    InstanceMethod(
      "dispatchIndirect",
      &GPUComputePassEncoder::dispatchIndirect,
      napi_enumerable
    ),
    InstanceMethod(
      "endPass",
      &GPUComputePassEncoder::endPass,
      napi_enumerable
    ),
    InstanceMethod(
      "setBindGroup",
      &GPUComputePassEncoder::setBindGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "pushDebugGroup",
      &GPUComputePassEncoder::pushDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "popDebugGroup",
      &GPUComputePassEncoder::popDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "insertDebugMarker",
      &GPUComputePassEncoder::insertDebugMarker,
      napi_enumerable
    ),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUComputePassEncoder", func);
  return exports;
}
