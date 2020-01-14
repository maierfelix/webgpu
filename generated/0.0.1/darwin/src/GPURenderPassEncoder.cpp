#include "GPURenderPassEncoder.h"
#include "GPUDevice.h"
#include "GPUCommandEncoder.h"
#include "GPURenderPipeline.h"
#include "GPUBuffer.h"
#include "GPUBindGroup.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPURenderPassEncoder::constructor;

GPURenderPassEncoder::GPURenderPassEncoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURenderPassEncoder>(info) {
  Napi::Env env = info.Env();

  this->commandEncoder.Reset(info[0].As<Napi::Object>(), 1);
  GPUCommandEncoder* commandEncoder = Napi::ObjectWrap<GPUCommandEncoder>::Unwrap(this->commandEncoder.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(commandEncoder->device.Value());

  auto descriptor = DescriptorDecoder::GPURenderPassDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuCommandEncoderBeginRenderPass(commandEncoder->instance, &descriptor);
}

GPURenderPassEncoder::~GPURenderPassEncoder() {
  this->device.Reset();
  this->commandEncoder.Reset();
  wgpuRenderPassEncoderRelease(this->instance);
}

Napi::Value GPURenderPassEncoder::setPipeline(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPURenderPipeline* renderPipeline = Napi::ObjectWrap<GPURenderPipeline>::Unwrap(info[0].As<Napi::Object>());

  wgpuRenderPassEncoderSetPipeline(this->instance, renderPipeline->instance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setIndexBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUBuffer* buffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t offset = 0;
  if (info[1].IsNumber()) {
    offset = static_cast<uint64_t>(info[1].As<Napi::Number>().Uint32Value());
  }

  wgpuRenderPassEncoderSetIndexBuffer(this->instance, buffer->instance, offset);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setVertexBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t startSlot = info[0].As<Napi::Number>().Uint32Value();
  WGPUBuffer buffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[1].As<Napi::Object>())->instance;
  uint32_t offset = 0;
  if (info[2].IsNumber()) {
    offset = info[2].As<Napi::Number>().Uint32Value();
  }

  wgpuRenderPassEncoderSetVertexBuffer(this->instance, startSlot, buffer, offset);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::draw(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t vertexCount = info[0].As<Napi::Number>().Uint32Value();
  uint32_t instanceCount = info[1].As<Napi::Number>().Uint32Value();
  uint32_t firstVertex = info[2].As<Napi::Number>().Uint32Value();
  uint32_t firstInstance = info[3].As<Napi::Number>().Uint32Value();

  wgpuRenderPassEncoderDraw(this->instance, vertexCount, instanceCount, firstVertex, firstInstance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::drawIndexed(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t indexCount = info[0].As<Napi::Number>().Uint32Value();
  uint32_t instanceCount = info[1].As<Napi::Number>().Uint32Value();
  uint32_t firstIndex = info[2].As<Napi::Number>().Uint32Value();
  int32_t baseVertex = info[3].As<Napi::Number>().Int32Value();
  uint32_t firstInstance = info[4].As<Napi::Number>().Uint32Value();

  wgpuRenderPassEncoderDrawIndexed(this->instance, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::drawIndirect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUBuffer* indirectBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t indirectOffset = static_cast<uint64_t>(info[1].As<Napi::Number>().Uint32Value());

  wgpuRenderPassEncoderDrawIndirect(this->instance, indirectBuffer->instance, indirectOffset);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::drawIndexedIndirect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUBuffer* indirectBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t indirectOffset = static_cast<uint64_t>(info[1].As<Napi::Number>().Uint32Value());

  wgpuRenderPassEncoderDrawIndexedIndirect(this->instance, indirectBuffer->instance, indirectOffset);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setViewport(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  float x = info[0].As<Napi::Number>().FloatValue();
  float y = info[1].As<Napi::Number>().FloatValue();
  float width = info[2].As<Napi::Number>().FloatValue();
  float height = info[3].As<Napi::Number>().FloatValue();
  float minDepth = info[4].As<Napi::Number>().FloatValue();
  float maxDepth = info[5].As<Napi::Number>().FloatValue();

  wgpuRenderPassEncoderSetViewport(this->instance, x, y, width, height, minDepth, maxDepth);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setScissorRect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t x = info[0].As<Napi::Number>().Uint32Value();
  uint32_t y = info[1].As<Napi::Number>().Uint32Value();
  uint32_t width = info[2].As<Napi::Number>().Uint32Value();
  uint32_t height = info[3].As<Napi::Number>().Uint32Value();

  wgpuRenderPassEncoderSetScissorRect(this->instance, x, y, width, height);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setBlendColor(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUCommandEncoder* commandEncoder = Napi::ObjectWrap<GPUCommandEncoder>::Unwrap(this->commandEncoder.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(commandEncoder->device.Value());

  auto color = DescriptorDecoder::GPUColor(device, info[0].As<Napi::Value>());

  wgpuRenderPassEncoderSetBlendColor(this->instance, &color);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setStencilReference(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t reference = info[0].As<Napi::Number>().Uint32Value();

  wgpuRenderPassEncoderSetStencilReference(this->instance, reference);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::executeBundles(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  // TODO
  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::endPass(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuRenderPassEncoderEndPass(this->instance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setBindGroup(const Napi::CallbackInfo &info) {
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

  wgpuRenderPassEncoderSetBindGroup(this->instance, groupIndex, group, dynamicOffsetCount, dynamicOffsets.data());

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::pushDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuRenderPassEncoderPushDebugGroup(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::popDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuRenderPassEncoderPopDebugGroup(this->instance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::insertDebugMarker(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuRenderPassEncoderInsertDebugMarker(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Object GPURenderPassEncoder::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURenderPassEncoder", {
    InstanceMethod(
      "setPipeline",
      &GPURenderPassEncoder::setPipeline,
      napi_enumerable
    ),
    InstanceMethod(
      "setIndexBuffer",
      &GPURenderPassEncoder::setIndexBuffer,
      napi_enumerable
    ),
    InstanceMethod(
      "setVertexBuffer",
      &GPURenderPassEncoder::setVertexBuffer,
      napi_enumerable
    ),
    InstanceMethod(
      "draw",
      &GPURenderPassEncoder::draw,
      napi_enumerable
    ),
    InstanceMethod(
      "drawIndexed",
      &GPURenderPassEncoder::drawIndexed,
      napi_enumerable
    ),
    InstanceMethod(
      "drawIndirect",
      &GPURenderPassEncoder::drawIndirect,
      napi_enumerable
    ),
    InstanceMethod(
      "drawIndexedIndirect",
      &GPURenderPassEncoder::drawIndexedIndirect,
      napi_enumerable
    ),
    InstanceMethod(
      "setViewport",
      &GPURenderPassEncoder::setViewport,
      napi_enumerable
    ),
    InstanceMethod(
      "setScissorRect",
      &GPURenderPassEncoder::setScissorRect,
      napi_enumerable
    ),
    InstanceMethod(
      "setBlendColor",
      &GPURenderPassEncoder::setBlendColor,
      napi_enumerable
    ),
    InstanceMethod(
      "setStencilReference",
      &GPURenderPassEncoder::setStencilReference,
      napi_enumerable
    ),
    InstanceMethod(
      "executeBundles",
      &GPURenderPassEncoder::executeBundles,
      napi_enumerable
    ),
    InstanceMethod(
      "endPass",
      &GPURenderPassEncoder::endPass,
      napi_enumerable
    ),
    InstanceMethod(
      "setBindGroup",
      &GPURenderPassEncoder::setBindGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "pushDebugGroup",
      &GPURenderPassEncoder::pushDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "popDebugGroup",
      &GPURenderPassEncoder::popDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "insertDebugMarker",
      &GPURenderPassEncoder::insertDebugMarker,
      napi_enumerable
    ),
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURenderPassEncoder", func);
  return exports;
}
