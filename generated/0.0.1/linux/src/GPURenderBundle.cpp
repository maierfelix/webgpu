#include "GPURenderBundle.h"

Napi::FunctionReference GPURenderBundle::constructor;

GPURenderBundle::GPURenderBundle(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPURenderBundle>(info) {

}

GPURenderBundle::~GPURenderBundle() {
  wgpuRenderBundleRelease(this->instance);
}

Napi::Object GPURenderBundle::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPURenderBundle", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPURenderBundle", func);
  return exports;
}
