#include "GPURenderPassEncoder.h"
#include "GPUDevice.h"
#include "GPUCommandEncoder.h"
#include "GPURenderPipeline.h"
#include "GPUBuffer.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPURenderPassEncoder::constructor;

GPURenderPassEncoder::GPURenderPassEncoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURenderPassEncoder>(info) {
  Napi::Env env = info.Env();

  this->commandEncoder.Reset(info[0].As<Napi::Object>(), 1);
  GPUCommandEncoder* commandEncoder = Napi::ObjectWrap<GPUCommandEncoder>::Unwrap(this->commandEncoder.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(commandEncoder->device.Value());

  DawnRenderPassDescriptor descriptor = DescriptorDecoder::GPURenderPassDescriptor(device, info[1].As<Napi::Value>());

  this->instance = dawnCommandEncoderBeginRenderPass(commandEncoder->instance, &descriptor);
}

GPURenderPassEncoder::~GPURenderPassEncoder() {
  // destructor
}

Napi::Value GPURenderPassEncoder::setPipeline(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPURenderPipeline* renderPipeline = Napi::ObjectWrap<GPURenderPipeline>::Unwrap(info[0].As<Napi::Object>());

  dawnRenderPassEncoderSetPipeline(this->instance, renderPipeline->instance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setIndexBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUBuffer* buffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t offset = 0;
  if (info[1].IsBigInt()) {
    bool lossless;
    offset = info[1].As<Napi::BigInt>().Uint64Value(&lossless);
  }

  dawnRenderPassEncoderSetIndexBuffer(this->instance, buffer->instance, offset);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setVertexBuffers(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t startSlot = info[0].As<Napi::Number>().Uint32Value();

  uint32_t count;
  std::vector<DawnBuffer> buffers;
  {
    Napi::Array array = info[1].As<Napi::Array>();
    count = array.Length();
    for (unsigned int ii = 0; ii < count; ++ii) {
      Napi::Object item = array.Get(ii).As<Napi::Object>();
      DawnBuffer buffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(item)->instance;
      buffers.push_back(buffer);
    };
  }

  std::vector<uint64_t> offsets;
  {
    Napi::Array array = info[2].As<Napi::Array>();
    for (unsigned int ii = 0; ii < count; ++ii) {
      bool lossless;
      uint64_t offset = array.Get(ii).As<Napi::BigInt>().Uint64Value(&lossless);
      offsets.push_back(offset);
    };
  }

  dawnRenderPassEncoderSetVertexBuffers(this->instance, startSlot, count, buffers.data(), offsets.data());

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::draw(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t vertexCount = info[0].As<Napi::Number>().Uint32Value();
  uint32_t instanceCount = info[1].As<Napi::Number>().Uint32Value();
  uint32_t firstVertex = info[2].As<Napi::Number>().Uint32Value();
  uint32_t firstInstance = info[3].As<Napi::Number>().Uint32Value();

  dawnRenderPassEncoderDraw(this->instance, vertexCount, instanceCount, firstVertex, firstInstance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::drawIndexed(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t indexCount = info[0].As<Napi::Number>().Uint32Value();
  uint32_t instanceCount = info[1].As<Napi::Number>().Uint32Value();
  uint32_t firstIndex = info[2].As<Napi::Number>().Uint32Value();
  int32_t baseVertex = info[3].As<Napi::Number>().Int32Value();
  uint32_t firstInstance = info[4].As<Napi::Number>().Uint32Value();

  dawnRenderPassEncoderDrawIndexed(this->instance, indexCount, instanceCount, firstIndex, baseVertex, firstInstance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::drawIndirect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  bool lossless;

  GPUBuffer* indirectBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t indirectOffset = info[1].As<Napi::BigInt>().Uint64Value(&lossless);

  dawnRenderPassEncoderDrawIndirect(this->instance, indirectBuffer->instance, indirectOffset);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::drawIndexedIndirect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  bool lossless;

  GPUBuffer* indirectBuffer = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>());
  uint64_t indirectOffset = info[1].As<Napi::BigInt>().Uint64Value(&lossless);

  dawnRenderPassEncoderDrawIndexedIndirect(this->instance, indirectBuffer->instance, indirectOffset);

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

  dawnRenderPassEncoderSetViewport(this->instance, x, y, width, height, minDepth, maxDepth);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setScissorRect(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t x = info[0].As<Napi::Number>().Uint32Value();
  uint32_t y = info[1].As<Napi::Number>().Uint32Value();
  uint32_t width = info[2].As<Napi::Number>().Uint32Value();
  uint32_t height = info[3].As<Napi::Number>().Uint32Value();

  dawnRenderPassEncoderSetScissorRect(this->instance, x, y, width, height);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setBlendColor(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUCommandEncoder* commandEncoder = Napi::ObjectWrap<GPUCommandEncoder>::Unwrap(this->commandEncoder.Value());
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(commandEncoder->device.Value());

  DawnColor color = DescriptorDecoder::GPUColor(device, info[0].As<Napi::Value>());

  dawnRenderPassEncoderSetBlendColor(this->instance, &color);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::setStencilReference(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  uint32_t reference = info[0].As<Napi::Number>().Uint32Value();

  dawnRenderPassEncoderSetStencilReference(this->instance, reference);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::executeBundles(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  // TODO
  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::endPass(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  dawnRenderPassEncoderEndPass(this->instance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::pushDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  dawnRenderPassEncoderPushDebugGroup(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::popDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  dawnRenderPassEncoderPopDebugGroup(this->instance);

  return env.Undefined();
}

Napi::Value GPURenderPassEncoder::insertDebugMarker(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  dawnRenderPassEncoderInsertDebugMarker(this->instance, groupLabel);

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
      "setVertexBuffers",
      &GPURenderPassEncoder::setVertexBuffers,
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
