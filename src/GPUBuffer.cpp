#include "GPUBuffer.h"
#include "GPUDevice.h"

#include <thread>
#include <chrono>
#include <cstdint>

#include <iostream>

Napi::FunctionReference GPUBuffer::constructor;

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

  intptr_t bufferAddr;

  dawnBufferMapReadAsync(
    this->buffer,
    [](DawnBufferMapAsyncStatus status, const void* data, uint64_t dataLength, void* userdata) {
      std::cout << 1337 << "::" << dataLength << std::endl;
      *(reinterpret_cast<intptr_t*>(userdata)) = reinterpret_cast<intptr_t>(data);
    },
    &bufferAddr
  );

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  DawnDevice backendDevice = device->backendDevice;

  dawnDeviceTick(backendDevice);
  if (bufferAddr == 0) {
    while (bufferAddr == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      dawnDeviceTick(backendDevice);
    };
  }

  callback.Call({ });

  return env.Undefined();
}

Napi::Value GPUBuffer::mapWriteAsync(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
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
      "mapWriteAsync",
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
