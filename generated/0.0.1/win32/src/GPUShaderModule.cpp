#include "GPUShaderModule.h"
#include "GPUDevice.h"

#include <vector>

#include <shaderc/shaderc.hpp>

Napi::FunctionReference GPUShaderModule::constructor;

GPUShaderModule::GPUShaderModule(const Napi::CallbackInfo& info) : Napi::ObjectWrap<GPUShaderModule>(info) {
  Napi::Env env = info.Env();

  this->device.Reset(info[0].As<Napi::Object>(), 1);
  GPUDevice* uwDevice = Napi::ObjectWrap<GPUDevice>::Unwrap(this->device.Value());
  WGPUDevice backendDevice = uwDevice->instance;

  WGPUShaderModuleDescriptor descriptor;
  descriptor.nextInChain = nullptr;

  {
    Napi::Object obj = info[1].As<Napi::Object>();
    Napi::Value code = obj.Get("code");
    // code is 'String'
    if (code.IsString()) {
      // shaderc inputs
      const char* source = getNAPIStringCopy(code);
      size_t size = strlen(source);
      shaderc_shader_kind kind = shaderc_glsl_infer_from_source;

      shaderc::Compiler compiler;

      shaderc::CompilationResult<uint32_t> result = compiler.CompileGlslToSpv(source, size, kind, "shader");

      if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        // create copy
        uwDevice->throwCallbackError(
          Napi::String::New(env, "Error"),
          Napi::String::New(env, result.GetErrorMessage())
        );
        return;
      }

      const uint32_t* resultBegin = result.cbegin();
      const uint32_t* resultEnd = result.cend();

      ptrdiff_t resultSize = resultEnd - resultBegin;

      descriptor.code = result.cbegin();
      descriptor.codeSize = static_cast<uint32_t>(resultSize);
      this->instance = wgpuDeviceCreateShaderModule(backendDevice, &descriptor);
      delete source;
    }
    // code is 'Uint32Array'
    else if (code.IsTypedArray()) {
      if (code.As<Napi::TypedArray>().TypedArrayType() == napi_uint32_array) {
        size_t size;
        descriptor.code = getTypedArrayData<uint32_t>(code, &size);
        descriptor.codeSize = static_cast<uint32_t>(size);
        this->instance = wgpuDeviceCreateShaderModule(backendDevice, &descriptor);
      } else {
        uwDevice->throwCallbackError(
          Napi::String::New(env, "TypeError"),
          Napi::String::New(env, "Expected 'Uint32Array' for property 'code'")
        );
      }
    }
    // code is invalid type
    else {
      uwDevice->throwCallbackError(
        Napi::String::New(env, "TypeError"),
        Napi::String::New(env, "Expected 'String' or 'Uint32Array' property 'code'")
      );
    }
  }

}

GPUShaderModule::~GPUShaderModule() {
  this->device.Reset();
  wgpuShaderModuleRelease(this->instance);
}

Napi::Object GPUShaderModule::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "GPUShaderModule", {

  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("GPUShaderModule", func);
  return exports;
}
