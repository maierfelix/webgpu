#include "GPUAdapter.h"

Napi::FunctionReference GPUAdapter::constructor;

GPUAdapter::GPUAdapter(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUAdapter>(info) {
  Napi::Env env = info.Env();

  this->window = this->createWindow(info);

  this->nativeInstance = std::make_unique<dawn_native::Instance>();

  this->nativeInstance->DiscoverDefaultAdapters();

  this->instance = this->createAdapter(info);
}

GPUAdapter::~GPUAdapter() {
  // destructor
}

Napi::Value GPUAdapter::requestDevice(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();

  Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(env);

  std::vector<napi_value> args = {};
  args.push_back(info.This().As<Napi::Value>());

  // process arguments
  if (info[0].IsObject()) {
    args.push_back(info[0].As<Napi::Value>());
  }

  Napi::Object device = GPUDevice::constructor.New(args);
  deferred.Resolve(device);

  return deferred.Promise();
}

GLFWwindow* GPUAdapter::createWindow(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!glfwInit()) {
    Napi::Error::New(env, "Failed to initialise GLFW").ThrowAsJavaScriptException();
    return nullptr;
  }
  // backend dependant hints
  {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }
  GLFWwindow* window = glfwCreateWindow(640, 480, "NWGPU", nullptr, nullptr);
  if (!window) {
    Napi::Error::New(env, "Failed to create window").ThrowAsJavaScriptException();
    return nullptr;
  }
  return window;
}

dawn_native::Adapter GPUAdapter::createAdapter(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  {
    std::vector<dawn_native::Adapter> adapters = this->nativeInstance->GetAdapters();
    auto adapterIt = std::find_if(
      adapters.begin(),
      adapters.end(),
      [](const dawn_native::Adapter adapter) -> bool {
        return adapter.GetBackendType() == dawn_native::BackendType::D3D12;
      }
    );
    if (adapterIt == adapters.end()) {
      Napi::Error::New(env, "No compatible adapter found").ThrowAsJavaScriptException();
      return nullptr;
    }
    return *adapterIt;
  }
}

Napi::Value GPUAdapter::GetName(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::String::New(env, this->instance.GetPCIInfo().name);
}

Napi::Value GPUAdapter::GetExtensions(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();

  std::vector<const char*> extensions = this->instance.GetSupportedExtensions();

  Napi::Array out = Napi::Array::New(env);
  for (unsigned int ii = 0; ii < extensions.size(); ++ii) {
    out[ii] = Napi::String::New(env, extensions.at(ii));
  };

  return out;
}

Napi::Object GPUAdapter::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUAdapter", {
    InstanceAccessor(
      "name",
      &GPUAdapter::GetName,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "extensions",
      &GPUAdapter::GetExtensions,
      nullptr,
      napi_enumerable
    ),
    InstanceMethod(
      "_requestDevice",
      &GPUAdapter::requestDevice,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUAdapter", func);
  return exports;
}
