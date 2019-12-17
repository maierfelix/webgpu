#include "GPUCommandEncoder.h"
#include "GPUDevice.h"
#include "GPUBuffer.h"
#include "GPUCommandBuffer.h"
#include "GPURenderPassEncoder.h"
#include "GPUComputePassEncoder.h"
#include "GPURayTracingAccelerationContainer.h"

#include "DescriptorDecoder.h"

Napi::FunctionReference GPUCommandEncoder::constructor;

GPUCommandEncoder::GPUCommandEncoder(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUCommandEncoder>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto descriptor = DescriptorDecoder::GPUCommandEncoderDescriptor(device, info[1].As<Napi::Value>());

  this->instance = wgpuDeviceCreateCommandEncoder(device->instance, &descriptor);
}

GPUCommandEncoder::~GPUCommandEncoder() {
  this->device.Reset();
  wgpuCommandEncoderRelease(this->instance);
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

Napi::Value GPUCommandEncoder::buildRayTracingAccelerationContainer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  WGPUCommandEncoder commandEncoder = this->instance;

  WGPURayTracingAccelerationContainer container = Napi::ObjectWrap<GPURayTracingAccelerationContainer>::Unwrap(
    info[0].As<Napi::Object>()
  )->instance;
  bool update = info[1].IsBoolean() ? info[0].As<Napi::Boolean>().Value() : false;

  wgpuCommandEncoderBuildRayTracingAccelerationContainer(commandEncoder, container, update);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyBufferToBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  WGPUCommandEncoder commandEncoder = this->instance;
  WGPUBuffer source = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[0].As<Napi::Object>())->instance;

  uint64_t sourceOffset = static_cast<uint64_t>(info[1].As<Napi::Number>().Uint32Value());

  WGPUBuffer destination = Napi::ObjectWrap<GPUBuffer>::Unwrap(info[2].As<Napi::Object>())->instance;
  uint64_t destinationOffset = static_cast<uint64_t>(info[3].As<Napi::Number>().Uint32Value());
  uint64_t size = static_cast<uint64_t>(info[4].As<Napi::Number>().Uint32Value());

  wgpuCommandEncoderCopyBufferToBuffer(commandEncoder, source, sourceOffset, destination, destinationOffset, size);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyBufferToTexture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto source = DescriptorDecoder::GPUBufferCopyView(device, info[0].As<Napi::Value>());
  auto destination = DescriptorDecoder::GPUTextureCopyView(device, info[1].As<Napi::Value>());
  auto copySize = DescriptorDecoder::GPUExtent3D(device, info[2].As<Napi::Value>());

  wgpuCommandEncoderCopyBufferToTexture(this->instance, &source, &destination, &copySize);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyTextureToBuffer(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto source = DescriptorDecoder::GPUTextureCopyView(device, info[0].As<Napi::Value>());
  auto destination = DescriptorDecoder::GPUBufferCopyView(device, info[1].As<Napi::Value>());
  auto copySize = DescriptorDecoder::GPUExtent3D(device, info[2].As<Napi::Value>());

  wgpuCommandEncoderCopyTextureToBuffer(this->instance, &source, &destination, &copySize);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyTextureToTexture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  auto source = DescriptorDecoder::GPUTextureCopyView(device, info[0].As<Napi::Value>());
  auto destination = DescriptorDecoder::GPUTextureCopyView(device, info[1].As<Napi::Value>());
  auto copySize = DescriptorDecoder::GPUExtent3D(device, info[2].As<Napi::Value>());

  wgpuCommandEncoderCopyTextureToTexture(this->instance, &source, &destination, &copySize);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::copyImageBitmapToTexture(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  Napi::String type = Napi::String::New(env, "Type");
  Napi::String message = Napi::String::New(env, "Unimplemented method 'GPUCommandEncoder::copyImageBitmapToTexture'");
  device->throwCallbackError(type, message);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::pushDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuCommandEncoderPushDebugGroup(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::popDebugGroup(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  wgpuCommandEncoderPopDebugGroup(this->instance);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::insertDebugMarker(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  const char* groupLabel = info[0].As<Napi::String>().Utf8Value().c_str();
  wgpuCommandEncoderInsertDebugMarker(this->instance, groupLabel);

  return env.Undefined();
}

Napi::Value GPUCommandEncoder::finish(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  WGPUCommandBuffer buffer = wgpuCommandEncoderFinish(this->instance, nullptr);

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
      "buildRayTracingAccelerationContainer",
      &GPUCommandEncoder::buildRayTracingAccelerationContainer,
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
