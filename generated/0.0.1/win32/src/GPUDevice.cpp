#include "GPUDevice.h"
#include "GPUAdapter.h"
#include "GPUQueue.h"

Napi::FunctionReference GPUDevice::constructor;

GPUDevice::GPUDevice(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUDevice>(info) {
  Napi::Env env = info.Env();

  // expect arg 0 be GPUAdapter
  this->adapter.Reset(info[0].ToObject(), 1);
  this->_adapter = Napi::ObjectWrap<GPUAdapter>::Unwrap(this->adapter.Value())->adapter;

  if (info[1].IsObject()) {
    Napi::Object obj = info[1].As<Napi::Object>();
    if (obj.Has(Napi::String::New(env, "extensions"))) {
      this->extensions.Reset(obj.Get("extensions").As<Napi::Object>(), 1);
    }
    if (obj.Has(Napi::String::New(env, "limits"))) {
      this->limits.Reset(obj.Get("limits").As<Napi::Object>(), 1);
    }
  }

  this->backendDevice = this->createDevice();
  this->binding = this->createBinding(info, this->backendDevice);

  DawnProcTable procs = dawn_native::GetProcs();

  dawnSetProcs(&procs);
  //procs.deviceSetUncapturedErrorCallback(this->backendDevice, onDeviceError, nullptr);
  this->device = dawn::Device::Acquire(this->backendDevice);

  this->mainQueue.Reset(this->createQueue(info), 1);
}

GPUDevice::~GPUDevice() {
  // destructor
}

DawnDevice GPUDevice::createDevice() {
  DawnDevice device = this->_adapter.CreateDevice();
  return device;
}

BackendBinding* GPUDevice::createBinding(const Napi::CallbackInfo& info, DawnDevice device) {
  Napi::Env env = info.Env();
  dawn_native::BackendType backendType = this->_adapter.GetBackendType();
  BackendBinding* binding = CreateBinding(backendType, nullptr, device);
  if (binding == nullptr) {
    Napi::Error::New(env, "Failed to create binding backend").ThrowAsJavaScriptException();
  }
  return binding;
}

Napi::Object GPUDevice::createQueue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::vector<napi_value> args = {
    info.This().As<Napi::Value>()
  };
  Napi::Object queue = GPUQueue::constructor.New(args);
  return queue;
}

Napi::Value GPUDevice::GetExtensions(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->extensions.IsEmpty()) return env.Null();
  return this->extensions.Value().As<Napi::Object>();
}

Napi::Value GPUDevice::GetLimits(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->extensions.IsEmpty()) return env.Null();
  return this->extensions.Value().As<Napi::Object>();
}

Napi::Value GPUDevice::GetAdapter(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return this->adapter.Value().As<Napi::Object>();
}

Napi::Value GPUDevice::tick(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  this->device.Tick();
  return env.Undefined();
}

Napi::Value GPUDevice::getQueue(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return this->mainQueue.Value().As<Napi::Object>();
}

Napi::Object GPUDevice::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUDevice", {
    InstanceAccessor(
      "extensions",
      &GPUDevice::GetExtensions,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "limits",
      &GPUDevice::GetLimits,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "adapter",
      &GPUDevice::GetAdapter,
      nullptr,
      napi_enumerable
    ),
    InstanceMethod(
      "getQueue",
      &GPUDevice::getQueue,
      napi_enumerable
    ),
    InstanceMethod(
      "tick",
      &GPUDevice::tick,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUDevice", func);
  return exports;
}
