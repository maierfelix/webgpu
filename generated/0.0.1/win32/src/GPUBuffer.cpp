#include "GPUBuffer.h"
#include "GPUDevice.h"

#include <thread>
#include <chrono>
#include <cstdint>

#include <iostream>

Napi::FunctionReference GPUBuffer::constructor;

struct BufferCallbackResult {
  void* addr = nullptr;
  uint64_t length = 0;
};

GPUBuffer::GPUBuffer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUBuffer>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  DawnDevice backendDevice = device->backendDevice;

  DawnBufferDescriptor descriptor;
  {
    Napi::Object obj = info[1].As<Napi::Object>();
    bool lossless;
    descriptor.size = obj.Get("size").As<Napi::BigInt>().Uint64Value(&lossless);
    descriptor.usage = static_cast<DawnBufferUsage>(obj.Get("usage").As<Napi::Number>().Uint32Value());
  }

  this->buffer = dawnDeviceCreateBuffer(backendDevice, &descriptor);
}

GPUBuffer::~GPUBuffer() {
  // destructor
}

Napi::Value GPUBuffer::mapReadAsync(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Napi::Function callback = info[0].As<Napi::Function>();

  BufferCallbackResult callbackResult;

  dawnBufferMapReadAsync(
    this->buffer,
    [](DawnBufferMapAsyncStatus status, const void* data, uint64_t dataLength, void* userdata) {
      (*reinterpret_cast<BufferCallbackResult*>(userdata)) = { const_cast<void*>(data), dataLength };
    },
    &callbackResult
  );

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  DawnDevice backendDevice = device->backendDevice;

  dawnDeviceTick(backendDevice);
  if (!callbackResult.addr) {
    while (!callbackResult.addr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      dawnDeviceTick(backendDevice);
    };
  }

  Napi::ArrayBuffer buffer = Napi::ArrayBuffer::New(
    env,
    callbackResult.addr,
    callbackResult.length
  );

  callback.Call({ buffer });

  return env.Undefined();
}

Napi::Value GPUBuffer::mapWriteAsync(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Napi::Function callback = info[0].As<Napi::Function>();

  BufferCallbackResult callbackResult;
  dawnBufferMapWriteAsync(
    this->buffer,
    [](DawnBufferMapAsyncStatus status, void* ptr, uint64_t dataLength, void* userdata) {
      (*reinterpret_cast<BufferCallbackResult*>(userdata)) = { ptr, dataLength };
    },
    &callbackResult
  );

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  DawnDevice backendDevice = device->backendDevice;

  dawnDeviceTick(backendDevice);
  if (!callbackResult.addr) {
    while (!callbackResult.addr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      dawnDeviceTick(backendDevice);
    };
  }

  Napi::ArrayBuffer buffer = Napi::ArrayBuffer::New(
    env,
    callbackResult.addr,
    callbackResult.length
  );

  callback.Call({ buffer });

  return env.Undefined();
}

Napi::Value GPUBuffer::unmap(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  dawnBufferUnmap(this->buffer);
  return env.Undefined();
}

Napi::Value GPUBuffer::destroy(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  dawnBufferDestroy(this->buffer);
  return env.Undefined();
}

Napi::Object GPUBuffer::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUBuffer", {
    InstanceMethod(
      "_mapReadAsync",
      &GPUBuffer::mapReadAsync,
      napi_enumerable
    ),
    InstanceMethod(
      "_mapWriteAsync",
      &GPUBuffer::mapWriteAsync,
      napi_enumerable
    ),
    InstanceMethod(
      "unmap",
      &GPUBuffer::unmap,
      napi_enumerable
    ),
    InstanceMethod(
      "destroy",
      &GPUBuffer::destroy,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUBuffer", func);
  return exports;
}
