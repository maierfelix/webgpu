#include "WebGPUWindow.h"
#include "GPUCanvasContext.h"

Napi::FunctionReference WebGPUWindow::constructor;

WebGPUWindow::WebGPUWindow(const Napi::CallbackInfo& info) : Napi::ObjectWrap<WebGPUWindow>(info), env_(info.Env()) {
  Napi::Env env = env_;
  if (info.IsConstructCall()) {
    if (info[0].IsObject()) {
      // init glfw
      if (!glfwInit()) Napi::TypeError::New(env, "Failed to initialise GLFW").ThrowAsJavaScriptException();
      Napi::Object obj = info[0].As<Napi::Object>();
      if (!obj.Has("width"))  Napi::Error::New(env, "'Object' must contain property 'width'").ThrowAsJavaScriptException();
      if (!obj.Has("height")) Napi::Error::New(env, "'Object' must contain property 'height'").ThrowAsJavaScriptException();
      Napi::Value argWidth = obj.Get("width");
      Napi::Value argHeight = obj.Get("height");
      Napi::Value argTitle = obj.Has("title") ? obj.Get("title") : Napi::String::New(env, "");
      if (argWidth.IsNumber()) this->width = argWidth.As<Napi::Number>().Int32Value();
      if (argHeight.IsNumber()) this->height = argHeight.As<Napi::Number>().Int32Value();
      if (argTitle.IsString()) this->title = argTitle.As<Napi::String>().Utf8Value();
      glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
      if (obj.Has("resizable") && obj.Get("resizable").IsBoolean()) {
        bool resizable = obj.Get("resizable").As<Napi::Boolean>().Value();
        glfwWindowHint(GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
      } else {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
      }
      if (obj.Has("visible") && obj.Get("visible").IsBoolean()) {
        bool visible = obj.Get("visible").As<Napi::Boolean>().Value();
        glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
      } else {
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
      }
      #ifdef __APPLE__
      glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
      #endif
      GLFWwindow* window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);
      this->instance = window;
      //glfwMakeContextCurrent(window);
      glfwSetWindowUserPointer(window, this);
      // window events
      glfwSetWindowSizeCallback(window, WebGPUWindow::onWindowResize);
      glfwSetWindowFocusCallback(window, WebGPUWindow::onWindowFocus);
      glfwSetWindowCloseCallback(window, WebGPUWindow::onWindowClose);
      // keyboard events
      glfwSetKeyCallback(window, WebGPUWindow::onWindowKeyPress);
      // mouse events
      glfwSetCursorPosCallback(window, WebGPUWindow::onWindowMouseMove);
      glfwSetScrollCallback(window, WebGPUWindow::onWindowMouseWheel);
      glfwSetMouseButtonCallback(window, WebGPUWindow::onWindowMouseButton);
      // file drop
      glfwSetDropCallback(window, WebGPUWindow::onWindowDrop);
    } else {
      Napi::Error::New(env, "Argument 1 must be of type 'Object'").ThrowAsJavaScriptException();
    }
  } else {
    Napi::Error::New(env, "WebGPUWindow constructor cannot be invoked without 'new'").ThrowAsJavaScriptException();
  }
}

WebGPUWindow::~WebGPUWindow() {
  this->onresize.Reset();
  this->onfocus.Reset();
  this->onclose.Reset();
  this->onkeydown.Reset();
  this->onkeyup.Reset();
  this->onmousemove.Reset();
  this->onmousewheel.Reset();
  this->onmousedown.Reset();
  this->onmouseup.Reset();
  this->ondrop.Reset();
}

void WebGPUWindow::onWindowResize(GLFWwindow* window, int width, int height) {
  WebGPUWindow* self = static_cast<WebGPUWindow*>(glfwGetWindowUserPointer(window));
  Napi::Env env = self->env_;
  if (width != self->width || height != self->height) {
    // reconfigurate swapchain
    GPUSwapChain* swapChain = self->swapChain;
    wgpuSwapChainConfigure(
      swapChain->instance,
      swapChain->format,
      swapChain->usage,
      width,
      height
    );
  }
  self->width = width;
  self->height = height;

  if (self->onresize.IsEmpty()) return;
  Napi::Object out = Napi::Object::New(env);
  out.Set("width", Napi::Number::New(env, self->width));
  out.Set("height", Napi::Number::New(env, self->height));
  self->onresize.Value().As<Napi::Function>()({ out });
}

void WebGPUWindow::onWindowFocus(GLFWwindow* window, int focused) {
  WebGPUWindow* self = static_cast<WebGPUWindow*>(glfwGetWindowUserPointer(window));
  Napi::Env env = self->env_;
  if (self->onfocus.IsEmpty()) return;
  Napi::Object out = Napi::Object::New(env);
  out.Set("focused", Napi::Boolean::New(env, !!focused));
  self->onfocus.Value().As<Napi::Function>()({ out });
}

void WebGPUWindow::onWindowClose(GLFWwindow* window) {
  WebGPUWindow* self = static_cast<WebGPUWindow*>(glfwGetWindowUserPointer(window));
  Napi::Env env = self->env_;
  if (!self->onclose.IsEmpty()) {
    Napi::Object out = Napi::Object::New(env);
    self->onclose.Value().As<Napi::Function>()({ out });
  }
  if (!self->isClosed) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    glfwDestroyWindow(window);
    self->isClosed = true;
  }
}

void WebGPUWindow::onWindowKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods) {
  WebGPUWindow* self = static_cast<WebGPUWindow*>(glfwGetWindowUserPointer(window));
  Napi::Env env = self->env_;
  Napi::Object out = Napi::Object::New(env);
  out.Set("keyCode", Napi::Number::New(env, key));
  // press
  if (action == GLFW_PRESS) {
    if (!(self->onkeydown.IsEmpty())) {
      self->onkeydown.Value().As<Napi::Function>()({ out });
    }
  }
  // release
  else if (action == GLFW_RELEASE) {
    if (!(self->onkeyup.IsEmpty())) {
      self->onkeyup.Value().As<Napi::Function>()({ out });
    }
  }
}

void WebGPUWindow::onWindowMouseMove(GLFWwindow* window, double x, double y) {
  WebGPUWindow* self = static_cast<WebGPUWindow*>(glfwGetWindowUserPointer(window));
  Napi::Env env = self->env_;
  Napi::Object out = Napi::Object::New(env);
  double movementX = self->mouseLastX - x;
  double movementY = self->mouseLastY - y;
  self->mouseLastX = x;
  self->mouseLastY = y;
  if (self->onmousemove.IsEmpty()) return;
  out.Set("x", Napi::Number::New(env, x));
  out.Set("y", Napi::Number::New(env, y));
  out.Set("movementX", Napi::Number::New(env, movementX));
  out.Set("movementY", Napi::Number::New(env, movementY));
  self->onmousemove.Value().As<Napi::Function>()({ out });
}

void WebGPUWindow::onWindowMouseWheel(GLFWwindow* window, double deltaX, double deltaY) {
  WebGPUWindow* self = static_cast<WebGPUWindow*>(glfwGetWindowUserPointer(window));
  Napi::Env env = self->env_;
  if (self->onmousewheel.IsEmpty()) return;
  double mouseX = 0;
  double mouseY = 0;
  glfwGetCursorPos(window, &mouseX, &mouseY);
  Napi::Object out = Napi::Object::New(env);
  out.Set("x", Napi::Number::New(env, mouseX));
  out.Set("y", Napi::Number::New(env, mouseY));
  out.Set("deltaX", Napi::Number::New(env, deltaX));
  out.Set("deltaY", Napi::Number::New(env, deltaY));
  self->onmousewheel.Value().As<Napi::Function>()({ out });
}

void WebGPUWindow::onWindowMouseButton(GLFWwindow* window, int button, int action, int mods) {
  WebGPUWindow* self = static_cast<WebGPUWindow*>(glfwGetWindowUserPointer(window));
  Napi::Env env = self->env_;
  Napi::Object out = Napi::Object::New(env);
  double mouseX = 0;
  double mouseY = 0;
  glfwGetCursorPos(window, &mouseX, &mouseY);
  out.Set("x", Napi::Number::New(env, mouseX));
  out.Set("y", Napi::Number::New(env, mouseY));
  out.Set("button", Napi::Number::New(env, button));
  // press
  if (action == GLFW_PRESS) {
    if (!(self->onmousedown.IsEmpty())) {
      self->onmousedown.Value().As<Napi::Function>()({ out });
    }
  }
  // release
  else if (action == GLFW_RELEASE) {
    if (!(self->onmouseup.IsEmpty())) {
      self->onmouseup.Value().As<Napi::Function>()({ out });
    }
  }
}

void WebGPUWindow::onWindowDrop(GLFWwindow* window, int count, const char** paths) {
  WebGPUWindow* self = static_cast<WebGPUWindow*>(glfwGetWindowUserPointer(window));
  Napi::Env env = self->env_;
  if (self->ondrop.IsEmpty()) return;
  Napi::Object out = Napi::Object::New(env);
  // fill paths
  Napi::Array arr = Napi::Array::New(env, count);
  unsigned int len = count;
  for (unsigned int ii = 0; ii < len; ++ii) {
    arr.Set(ii, Napi::String::New(env, paths[ii]));
  };
  // add to out obj
  out.Set("paths", arr);
  self->ondrop.Value().As<Napi::Function>()({ out });
}

Napi::Value WebGPUWindow::shouldClose(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  if (this->isClosed) Napi::Boolean::New(env, false);
  return Napi::Boolean::New(env, static_cast<bool>(glfwWindowShouldClose(window)));
}

Napi::Value WebGPUWindow::focus(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  if (!this->isClosed) glfwFocusWindow(window);
  return env.Undefined();
}

Napi::Value WebGPUWindow::close(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  if (!this->isClosed) WebGPUWindow::onWindowClose(window);
  return env.Undefined();
}

Napi::Value WebGPUWindow::getContext(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object canvasContext = GPUCanvasContext::constructor.New({
    info.This().As<Napi::Value>()
  });
  return canvasContext;
}

Napi::Value WebGPUWindow::pollEvents(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  if (!this->isClosed && !glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }
  return env.Undefined();
}

// title
Napi::Value WebGPUWindow::Gettitle(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::String str = Napi::String::New(env, this->title.c_str());
  return str;
}
void WebGPUWindow::Settitle(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (!value.IsString()) {
    Napi::TypeError::New(env, "Argument 1 must be of type 'String'").ThrowAsJavaScriptException();
  }
  std::string title = value.ToString().Utf8Value();
  if (!this->isClosed) glfwSetWindowTitle(this->instance, title.c_str());
  this->title = title;
}

// width
Napi::Value WebGPUWindow::Getwidth(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, static_cast<int32_t>(this->width));
}
void WebGPUWindow::Setwidth(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  if (!value.IsNumber()) {
    Napi::TypeError::New(env, "Argument 1 must be of type 'Number'").ThrowAsJavaScriptException();
  }
  this->width = value.As<Napi::Number>().Int32Value();
  if (!this->isClosed) {
    glfwSetWindowSize(window, this->width, this->height);
    WebGPUWindow::onWindowResize(window, this->width, this->height);
  }
}

// height
Napi::Value WebGPUWindow::Getheight(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  return Napi::Number::New(env, static_cast<int32_t>(this->height));
}
void WebGPUWindow::Setheight(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  if (!value.IsNumber()) {
    Napi::TypeError::New(env, "Argument 1 must be of type 'Number'").ThrowAsJavaScriptException();
  }
  this->height = value.As<Napi::Number>().Int32Value();
  if (!this->isClosed) {
    glfwSetWindowSize(window, this->width, this->height);
    WebGPUWindow::onWindowResize(window, this->width, this->height);
  }
}

// frameBufferWidth
Napi::Value WebGPUWindow::GetframeBufferWidth(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  int width = 0, height = 0;
  if (!this->isClosed) glfwGetFramebufferSize(window, &width, &height);
  return Napi::Number::New(env, static_cast<int32_t>(width));
}

// frameBufferHeight
Napi::Value WebGPUWindow::GetframeBufferHeight(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  int width = 0, height = 0;
  if (!this->isClosed) glfwGetFramebufferSize(window, &width, &height);
  return Napi::Number::New(env, static_cast<int32_t>(height));
}

// devicePixelRatio
Napi::Value WebGPUWindow::GetdevicePixelRatio(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  GLFWwindow* window = this->instance;
  int width = 0, height = 0;
  if (!this->isClosed) glfwGetFramebufferSize(window, &width, &height);
  return Napi::Number::New(env, static_cast<int32_t>(width / this->width));
}

// onresize
Napi::Value WebGPUWindow::Getonresize(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onresize.IsEmpty()) return env.Null();
  return this->onresize.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonresize(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onresize.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onresize.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// onfocus
Napi::Value WebGPUWindow::Getonfocus(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onfocus.IsEmpty()) return env.Null();
  return this->onfocus.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonfocus(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onfocus.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onfocus.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// onclose
Napi::Value WebGPUWindow::Getonclose(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onclose.IsEmpty()) return env.Null();
  return this->onclose.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonclose(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onclose.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onclose.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// onkeydown
Napi::Value WebGPUWindow::Getonkeydown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onkeydown.IsEmpty()) return env.Null();
  return this->onkeydown.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonkeydown(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onkeydown.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onkeydown.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// onkeyup
Napi::Value WebGPUWindow::Getonkeyup(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onkeyup.IsEmpty()) return env.Null();
  return this->onkeyup.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonkeyup(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onkeyup.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onkeyup.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// onmousemove
Napi::Value WebGPUWindow::Getonmousemove(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onmousemove.IsEmpty()) return env.Null();
  return this->onmousemove.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonmousemove(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onmousemove.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onmousemove.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// onmousewheel
Napi::Value WebGPUWindow::Getonmousewheel(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onmousewheel.IsEmpty()) return env.Null();
  return this->onmousewheel.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonmousewheel(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onmousewheel.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onmousewheel.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// onmousedown
Napi::Value WebGPUWindow::Getonmousedown(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onmousedown.IsEmpty()) return env.Null();
  return this->onmousedown.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonmousedown(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onmousedown.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onmousedown.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// onmouseup
Napi::Value WebGPUWindow::Getonmouseup(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->onmouseup.IsEmpty()) return env.Null();
  return this->onmouseup.Value().As<Napi::Function>();
}
void WebGPUWindow::Setonmouseup(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->onmouseup.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->onmouseup.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

// ondrop
Napi::Value WebGPUWindow::Getondrop(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (this->ondrop.IsEmpty()) return env.Null();
  return this->ondrop.Value().As<Napi::Function>();
}
void WebGPUWindow::Setondrop(const Napi::CallbackInfo& info, const Napi::Value& value) {
  Napi::Env env = info.Env();
  if (value.IsFunction()) this->ondrop.Reset(value.As<Napi::Function>(), 1);
  else if (value.IsNull()) this->ondrop.Reset();
  else Napi::TypeError::New(env, "Argument 1 must be of type 'Function'").ThrowAsJavaScriptException();
}

Napi::Object WebGPUWindow::Initialize(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Napi::Function func = DefineClass(env, "WebGPUWindow", {
    // methods
    InstanceMethod(
      "getContext",
      &WebGPUWindow::getContext,
      napi_enumerable
    ),
    InstanceMethod(
      "pollEvents",
      &WebGPUWindow::pollEvents,
      napi_enumerable
    ),
    InstanceMethod(
      "focus",
      &WebGPUWindow::focus,
      napi_enumerable
    ),
    InstanceMethod(
      "close",
      &WebGPUWindow::close,
      napi_enumerable
    ),
    InstanceMethod(
      "shouldClose",
      &WebGPUWindow::shouldClose,
      napi_enumerable
    ),
    // accessors
    InstanceAccessor(
      "title",
      &WebGPUWindow::Gettitle,
      &WebGPUWindow::Settitle,
      napi_enumerable
    ),
    InstanceAccessor(
      "width",
      &WebGPUWindow::Getwidth,
      &WebGPUWindow::Setwidth,
      napi_enumerable
    ),
    InstanceAccessor(
      "height",
      &WebGPUWindow::Getheight,
      &WebGPUWindow::Setheight,
      napi_enumerable
    ),
    InstanceAccessor(
      "frameBufferWidth",
      &WebGPUWindow::GetframeBufferWidth,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "frameBufferHeight",
      &WebGPUWindow::GetframeBufferHeight,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "devicePixelRatio",
      &WebGPUWindow::GetdevicePixelRatio,
      nullptr,
      napi_enumerable
    ),
    InstanceAccessor(
      "onresize",
      &WebGPUWindow::Getonresize,
      &WebGPUWindow::Setonresize,
      napi_enumerable
    ),
    InstanceAccessor(
      "onfocus",
      &WebGPUWindow::Getonfocus,
      &WebGPUWindow::Setonfocus,
      napi_enumerable
    ),
    InstanceAccessor(
      "onclose",
      &WebGPUWindow::Getonclose,
      &WebGPUWindow::Setonclose,
      napi_enumerable
    ),
    InstanceAccessor(
      "onkeydown",
      &WebGPUWindow::Getonkeydown,
      &WebGPUWindow::Setonkeydown,
      napi_enumerable
    ),
    InstanceAccessor(
      "onkeyup",
      &WebGPUWindow::Getonkeyup,
      &WebGPUWindow::Setonkeyup,
      napi_enumerable
    ),
    InstanceAccessor(
      "onmousemove",
      &WebGPUWindow::Getonmousemove,
      &WebGPUWindow::Setonmousemove,
      napi_enumerable
    ),
    InstanceAccessor(
      "onmousewheel",
      &WebGPUWindow::Getonmousewheel,
      &WebGPUWindow::Setonmousewheel,
      napi_enumerable
    ),
    InstanceAccessor(
      "onmousedown",
      &WebGPUWindow::Getonmousedown,
      &WebGPUWindow::Setonmousedown,
      napi_enumerable
    ),
    InstanceAccessor(
      "onmouseup",
      &WebGPUWindow::Getonmouseup,
      &WebGPUWindow::Setonmouseup,
      napi_enumerable
    ),
    InstanceAccessor(
      "ondrop",
      &WebGPUWindow::Getondrop,
      &WebGPUWindow::Setondrop,
      napi_enumerable
    )
  });
  constructor = Napi::Persistent(func);
  constructor.SuppressDestruct();
  exports.Set("WebGPUWindow", func);
  return exports;
}
