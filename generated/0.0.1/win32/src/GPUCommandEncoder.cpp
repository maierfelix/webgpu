#include "GPUCommandEncoder.h"
#include "GPUDevice.h"
#include "GPUBuffer.h"
#include "GPUCommandBuffer.h"
#include "GPURenderPassEncoder.h"
#include "GPUComputePassEncoder.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUCommandEncoder::constructor;

GPUCommandEncoder::GPUCommandEncoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUCommandEncoder>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPUCommandEncoderDescriptor(device, info[1].As<Napi::Value>());

  this->instance = dawnDeviceCreateCommandEncoder(device->instance, &descriptor);
}

GPUCommandEncoder::~GPUCommandEncoder() {
  // destructor
}

Napi::Value GPUCommandEncoder::beginRenderPass(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object renderPass = GPURenderPassEncoder::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  return renderPass;
}

Napi::Value GPUCommandEncoder::beginComputePass(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object computePass = GPUComputePassEncoder::constructor.New({
    info.This().As<Napi::Value>(),
    info[0].As<Napi::Value>()
  });
  return computePass;
}

Napi::Value GPUCommandEncoder::copyBufferToBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  DawnCommandEncoder commandEncoder = this->instance;
  DawnBuffer source = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>())->instance;

  bool lossless;

  uint64_t sourceOffset = info[1].As<Napi::BigInt>().Uint64Value(&lossless);

  DawnBuffer destination = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[2].As<Napi::Object>())->instance;
  uint64_t destinationOffset = info[3].As<Napi::BigInt>().Uint64Value(&lossless);
  uint64_t size = info[4].As<Napi::BigInt>().Uint64Value(&lossless);

  dawnCommandEncoderCopyBufferToBuffer(commandEncoder, source, sourceOffset, destination, destinationOffset, size);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyBufferToTexture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto source = DescriptorDecoder::GPUBufferCopyView(device, info[0].As<Napi::Value>());
  auto destination = DescriptorDecoder::GPUTextureCopyView(device, info[1].As<Napi::Value>());
  auto copySize = DescriptorDecoder::GPUExtent3D(device, info[2].As<Napi::Value>());

  dawnCommandEncoderCopyBufferToTexture(this->instance, &source, &destination, &copySize);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyTextureToBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto source = DescriptorDecoder::GPUTextureCopyView(device, info[0].As<Napi::Value>());
  auto destination = DescriptorDecoder::GPUBufferCopyView(device, info[1].As<Napi::Value>());
  auto copySize = DescriptorDecoder::GPUExtent3D(device, info[2].As<Napi::Value>());

  dawnCommandEncoderCopyTextureToBuffer(this->instance, &source, &destination, &copySize);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyTextureToTexture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto source = DescriptorDecoder::GPUTextureCopyView(device, info[0].As<Napi::Value>());
  auto destination = DescriptorDecoder::GPUTextureCopyView(device, info[1].As<Napi::Value>());
  auto copySize = DescriptorDecoder::GPUExtent3D(device, info[2].As<Napi::Value>());

  dawnCommandEncoderCopyTextureToTexture(this->instance, &source, &destination, &copySize);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyImageBitmapToTexture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  // TODO
  return env.Undefined();
}

Napi::Value GPUCommandEncoder::pushDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  dawnCommandEncoderPushDebugGroup(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::popDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  dawnCommandEncoderPopDebugGroup(this->instance);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::insertDebugMarker(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  dawnCommandEncoderInsertDebugMarker(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::finish(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  //GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  //DawnCommandBufferDescriptor descriptor = DescriptorDecoder::GPUCommandBufferDescriptor(device, info[0].As<Napi::Value>());

  DawnCommandBuffer buffer = dawnCommandEncoderFinish(this->instance, nullptr);

  Napi::Object commandBuffer = GPUCommandBuffer::constructor.New({});
  GPUCommandBuffer* uwCommandBuffer = Napi::ObjectWrap<GPUCommandBuffer>::Unwrap(commandBuffer);
  uwCommandBuffer->instance = buffer;

  return commandBuffer;
}

Napi::Object GPUCommandEncoder::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUCommandEncoder", {
    InstanceMethod(
      "beginRenderPass",
      &GPUCommandEncoder::beginRenderPass,
      napi_enumerable
    ),
    InstanceMethod(
      "beginComputePass",
      &GPUCommandEncoder::beginComputePass,
      napi_enumerable
    ),
    InstanceMethod(
      "copyBufferToBuffer",
      &GPUCommandEncoder::copyBufferToBuffer,
      napi_enumerable
    ),
    InstanceMethod(
      "copyBufferToTexture",
      &GPUCommandEncoder::copyBufferToTexture,
      napi_enumerable
    ),
    InstanceMethod(
      "copyTextureToBuffer",
      &GPUCommandEncoder::copyTextureToBuffer,
      napi_enumerable
    ),
    InstanceMethod(
      "copyTextureToTexture",
      &GPUCommandEncoder::copyTextureToTexture,
      napi_enumerable
    ),
    InstanceMethod(
      "copyImageBitmapToTexture",
      &GPUCommandEncoder::copyImageBitmapToTexture,
      napi_enumerable
    ),
    InstanceMethod(
      "pushDebugGroup",
      &GPUCommandEncoder::pushDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "popDebugGroup",
      &GPUCommandEncoder::popDebugGroup,
      napi_enumerable
    ),
    InstanceMethod(
      "insertDebugMarker",
      &GPUCommandEncoder::insertDebugMarker,
      napi_enumerable
    ),
    InstanceMethod(
      "finish",
      &GPUCommandEncoder::finish,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUCommandEncoder", func);
  return exports;
}
