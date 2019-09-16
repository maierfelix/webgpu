#ifndef __GPU_UTILS_H__
#define __GPU_UTILS_H__

#define NAPI_EXPERIMENTAL
#include <napi.h>

inline char* getNAPIStringCopy(Napi::Value& value) {
  std::string utf8 = value.ToString().Utf8Value();
  int len = utf8.length() + 1; // +1 NULL
  char *str = new char[len];
  strncpy(str, utf8.c_str(), len);
  return str;
};

template<typename T> inline T* getTypedArrayData(Napi::Value& value, size_t *len = nullptr) {
  T *data = nullptr;
  if (len) *len = 0;
  if (!value.IsTypedArray()) {
    Napi::Env env = value.Env();
    Napi::Error::New(env, "Argument must be a 'ArrayBufferView'").ThrowAsJavaScriptException();
    return data;
  }
  Napi::TypedArray arr = value.As<Napi::TypedArray>();
  Napi::ArrayBuffer buffer = arr.ArrayBuffer();
  if (len) *len = buffer.ByteLength() / sizeof(T);
  data = reinterpret_cast<T*>(buffer.Data());
  return data;
};

#endif
