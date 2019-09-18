#include "GPUBuffer.h"
#include "GPUDevice.h"

#include "DescriptorDecoder.h"

#include <thread>
#include <chrono>
#include <cstdint>

Napi::FunctionReference GPUBuffer::constructor;

struct BufferCallbackResult {
  void* addr = nullptr;
  uint64_t length = 0;
};

GPUBuffer::GPUBuffer(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUBuffer>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());

  DawnBufferDescriptor descriptor = DescriptorDecoder::GPUBufferDescriptor(device, info[1].As<Napi::Value>());

  this->instance = dawnDeviceCreateBuffer(device->instance, &descriptor);
}

GPUBuffer::~GPUBuffer() {
  // destructor
}

Napi::Value GPUBuffer::mapReadAsync(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  bool hasCallback = info[0].IsFunction();

  Napi::Function callback;
  if (hasCallback) callback = info[0].As<Napi::Function>();

  BufferCallbackResult callbackResult;

  dawnBufferMapReadAsync(
    this->instance,
    [](DawnBufferMapAsyncStatus status, const void* data, uint64_t dataLength, void* userdata) {
      (*reinterpret_cast<BufferCallbackResult*>(userdata)) = { const_cast<void*>(data), dataLength };
    },
    &callbackResult
  );

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  DawnDevice backendDevice = device->instance;

  dawnDeviceTick(backendDevice);
  if (!callbackResult.addr) {
    while (!callbackResult.addr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      dawnDeviceTick(backendDevice);
    };
  }

  // TODO: neuter arraybuffer on freeing
  Napi::ArrayBuffer buffer = Napi::ArrayBuffer::New(
    env,
    callbackResult.addr,
    callbackResult.length,
    [](Napi::Env, void* data) {
      delete[] static_cast<uint8_t*>(data);
    }
  );

  if (hasCallback) callback.Call({ buffer });

  return hasCallback ? env.Undefined() : buffer;
}

Napi::Value GPUBuffer::mapWriteAsync(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Napi::Function callback = info[0].As<Napi::Function>();

  BufferCallbackResult callbackResult;
  dawnBufferMapWriteAsync(
    this->instance,
    [](DawnBufferMapAsyncStatus status, void* ptr, uint64_t dataLength, void* userdata) {
      (*reinterpret_cast<BufferCallbackResult*>(userdata)) = { ptr, dataLength };
    },
    &callbackResult
  );

  GPUDevice* device = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  DawnDevice backendDevice = device->instance;

  dawnDeviceTick(backendDevice);
  if (!callbackResult.addr) {
    while (!callbackResult.addr) {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      dawnDeviceTick(backendDevice);
    };
  }

  // TODO: neuter arraybuffer on freeing
  Napi::ArrayBuffer buffer = Napi::ArrayBuffer::New(
    env,
    callbackResult.addr,
    callbackResult.length,
    [](Napi::Env, void* data) {
      delete[] static_cast<uint8_t*>(data);
    }
  );

  callback.Call({ buffer });

  return env.Undefined();
}

Napi::Value GPUBuffer::unmap(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  dawnBufferUnmap(this->instance);
  return env.Undefined();
}

Napi::Value GPUBuffer::destroy(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  dawnBufferDestroy(this->instance);
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
