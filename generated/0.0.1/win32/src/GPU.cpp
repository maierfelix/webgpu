#include "GPU.h"
#include "GPUAdapter.h"
#include "GPUCanvasContext.h"

Napi::FunctionReference GPU::constructor;

GPU::GPU(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPU>(info) { }
GPU::~GPU() { }

Napi::Value GPU::requestAdapter(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  auto deferred = Napi::Promise::Deferred::New(env);
  deferred.Resolve(GPUAdapter::constructor.New({ }));
  return deferred.Promise();
}

Napi::Value GPU::getContext(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object canvasContext = GPUCanvasContext::constructor.New({});
  return canvasContext;
}

Napi::Object GPU::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPU", {
    StaticMethod(
      "requestAdapter",
      &GPU::requestAdapter,
      napi_enumerable
    ),
    StaticMethod(
      "getContext",
      &GPU::getContext,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPU", func);
  return exports;
}
