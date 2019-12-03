#include "GPURenderBundleEncoder.h"
#include "GPUDevice.h"
#include "GPUCommandEncoder.h"
#include "GPURenderBundle.h"
#include "GPUBuffer.h"
#include "GPUBindGroup.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPURenderBundleEncoder::constructor;

GPURenderBundleEncoder::GPURenderBundleEncoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURenderBundleEncoder>(info) {
  Napi::Env env = info.Env();

  this->commandEncoder.Reset(info[0].As<Napi::Object>(), 1);
  GPUCommandEncoder* commandEncoder = Napi::ObjectWrap<GPUCommandEncoder>::Unwrap(this->commandEncoder.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(commandEncoder->device.Value());

  auto descriptor = DescriptorDecoder::GPURenderBundleEncoderDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuDeviceCreateRenderBundleEncoder(device->instance, &descriptor);
}

GPURenderBundleEncoder::~GPURenderBundleEncoder() {
  // destructor
}

Napi::Value GPURenderBundleEncoder::setPipeline(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPURenderPipeline* renderPipeline = Napi::ObjectWrap<GPURenderPipeline>::Unwrap(info[0].As<Napi::Object>());

  wgpuRenderBundleEncoderSetPipeline(this->instance, renderPipeline->instance);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::setIndexBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUBuffer* buffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t offset = 0;
  if (info[1].IsBigInt()) {
    bool lossless;
    offset = info[1].As<Napi::BigInt>().Uint64Value(&lossless);
  }

  wgpuRenderBundleEncoderSetIndexBuffer(this->instance, buffer->instance, offset);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::setVertexBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t startSlot = info[0].As<Napi::Number>().Uint32Value();
  WGPUBuffer buffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[1].As<Napi::Object>())->instance;
  uint32_t offset = 0;
  if (info[2].IsNumber()) {
    offset = info[1].As<Napi::Number>().Uint32Value();
  }

  wgpuRenderBundleEncoderSetVertexBuffer(this->instance, startSlot, buffer, offset);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::draw(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t vertexCount = info[0].As<Napi::Number>().Uint32Value();
  uint32_t instanceCount = info[1].As<Napi::Number>().Uint32Value();
  uint32_t firstVertex = info[2].As<Napi::Number>().Uint32Value();
  uint32_t firstInstance = info[3].As<Napi::Number>().Uint32Value();

  wgpuRenderBundleEncoderDraw(this->instance, vertexCount, instanceCount, firstVertex, firstInstance);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::drawIndexed(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t indexCount = info[0].As<Napi::Number>().Uint32Value();
  uint32_t instanceCount = info[1].As<Napi::Number>().Uint32Value();
  uint32_t firstIndex = info[2].As<Napi::Number>().Uint32Value();
  int32_t baseVertex = info[3].As<Napi::Number>().Int32Value();
  uint32_t firstInstance = info[4].As<Napi::Number>().Uint32Value();

  wgpuRenderBundleEncoderDrawIndexed(this->instance, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::drawIndirect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  bool lossless;

  GPUBuffer* indirectBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t indirectOffset = info[1].As<Napi::BigInt>().Uint64Value(&lossless);

  wgpuRenderBundleEncoderDrawIndirect(this->instance, indirectBuffer->instance, indirectOffset);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::drawIndexedIndirect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  bool lossless;

  GPUBuffer* indirectBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t indirectOffset = info[1].As<Napi::BigInt>().Uint64Value(&lossless);

  wgpuRenderBundleEncoderDrawIndexedIndirect(this->instance, indirectBuffer->instance, indirectOffset);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::finish(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUCommandEncoder* commandEncoder = Napi::ObjectWrap<GPUCommandEncoder>::Unwrap(this->commandEncoder.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(commandEncoder->device.Value());

  auto descriptor = DescriptorDecoder::GPURenderBundleDescriptor(device, info[1].As<Napi::Value>());

  wgpuRenderBundleEncoderFinish(this->instance, &descriptor);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::setBindGroup(const Napi::CallbackInfo &info) {
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

  wgpuRenderBundleEncoderSetBindGroup(this->instance, groupIndex, group, dynamicOffsetCount, dynamicOffsets.data());

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::pushDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuRenderBundleEncoderPushDebugGroup(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::popDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuRenderBundleEncoderPopDebugGroup(this->instance);

  return env.Undefined();
}

Napi::Value GPURenderBundleEncoder::insertDebugMarker(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuRenderBundleEncoderInsertDebugMarker(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Object GPURenderBundleEncoder::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURenderBundleEncoder", {
    InstanceMethod(
      "setPipeline",
      &GPURenderBundleEncoder::setPipeline,
      napi_enumerable
    ),
    InstanceMethod(
      "setIndexBuffer",
      &GPURenderBundleEncoder::setIndexBuffer,
      napi_enumerable
    ),
    InstanceMethod(
      "setVertexBuffer",
      &GPURenderBundleEncoder::setVertexBuffer,
      napi_enumerable
    ),
    InstanceMethod(
      "draw",
      &GPURenderBundleEncoder::draw,
      napi_enumerable
    ),
    InstanceMethod(
      "drawIndexed",
      &GPURenderBundleEncoder::drawIndexed,
      napi_enumerable
    ),
    InstanceMethod(
      "drawIndirect",
      &GPURenderBundleEncoder::drawIndirect,
      napi_enumerable
    ),
    InstanceMethod(
      "drawIndexedIndirect",
      &GPURenderBundleEncoder::drawIndexedIndirect,
      napi_enumerable
    ),
    InstanceMethod(
      "finish",
      &GPURenderBundleEncoder::finish,
      napi_enumerable
    ),
    InstanceMethod(
      "setBindGroup",
      &GPURenderBundleEncoder::setBindGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "pushDebugGroup",
      &GPURenderBundleEncoder::pushDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "popDebugGroup",
      &GPURenderBundleEncoder::popDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "insertDebugMarker",
      &GPURenderBundleEncoder::insertDebugMarker,
      napi_enumerable
    ),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURenderBundleEncoder", func);
  return exports;
}
