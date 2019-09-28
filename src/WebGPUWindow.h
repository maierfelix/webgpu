#ifndef __WEB_GPU_WINDOW_H__
#define __WEB_GPU_WINDOW_H__

#include "Base.h"
#include "GPUSwapChain.h"

class WebGPUWindow : public Napi::ObjectWrap<WebGPUWindow> {

  public:

    static Napi::Object Initialize(Napi::Env env, Napi::Object exports);
    static Napi::FunctionReference constructor;

    WebGPUWindow(const Napi::CallbackInfo &info);
    ~WebGPUWindow();

    int width = 480;
    int height = 320;
    std::string title = "undefined";

    double mouseLastX = 0;
    double mouseLastY = 0;

    bool isClosed = false;

    // event callbacks
    Napi::FunctionReference onresize;
    Napi::FunctionReference onfocus;
    Napi::FunctionReference onclose;
    Napi::FunctionReference onkeydown;
    Napi::FunctionReference onkeyup;
    Napi::FunctionReference onmousemove;
    Napi::FunctionReference onmousewheel;
    Napi::FunctionReference onmousedown;
    Napi::FunctionReference onmouseup;
    Napi::FunctionReference ondrop;

    Napi::Env env_;

    GLFWwindow* instance;

    GPUSwapChain* swapChain;
    DawnTextureFormat preferredSwapChainFormat = DAWN_TEXTURE_FORMAT_UNDEFINED;

    Napi::Value getContext(const Napi::CallbackInfo &info);
    Napi::Value pollEvents(const Napi::CallbackInfo &info);

    Napi::Value focus(const Napi::CallbackInfo &info);
    Napi::Value close(const Napi::CallbackInfo &info);
    Napi::Value shouldClose(const Napi::CallbackInfo &info);

    Napi::Value Gettitle(const Napi::CallbackInfo &info);
    void Settitle(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getwidth(const Napi::CallbackInfo &info);
    void Setwidth(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getheight(const Napi::CallbackInfo &info);
    void Setheight(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value GetframeBufferWidth(const Napi::CallbackInfo &info);
    Napi::Value GetframeBufferHeight(const Napi::CallbackInfo &info);

    Napi::Value GetdevicePixelRatio(const Napi::CallbackInfo &info);

    Napi::Value Getonresize(const Napi::CallbackInfo &info);
    void Setonresize(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getonfocus(const Napi::CallbackInfo &info);
    void Setonfocus(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getonclose(const Napi::CallbackInfo &info);
    void Setonclose(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getonkeydown(const Napi::CallbackInfo &info);
    void Setonkeydown(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getonkeyup(const Napi::CallbackInfo &info);
    void Setonkeyup(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getonmousemove(const Napi::CallbackInfo &info);
    void Setonmousemove(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getonmousewheel(const Napi::CallbackInfo &info);
    void Setonmousewheel(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getonmousedown(const Napi::CallbackInfo &info);
    void Setonmousedown(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getonmouseup(const Napi::CallbackInfo &info);
    void Setonmouseup(const Napi::CallbackInfo &info, const Napi::Value& value);

    Napi::Value Getondrop(const Napi::CallbackInfo &info);
    void Setondrop(const Napi::CallbackInfo &info, const Napi::Value& value);

    static void onWindowResize(GLFWwindow*, int, int);
    static void onWindowFocus(GLFWwindow*, int);
    static void onWindowClose(GLFWwindow*);
    static void onWindowKeyPress(GLFWwindow*, int, int, int, int);
    static void onWindowMouseMove(GLFWwindow*, double, double);
    static void onWindowMouseWheel(GLFWwindow*, double, double);
    static void onWindowMouseButton(GLFWwindow*, int, int, int);
    static void onWindowDrop(GLFWwindow*, int, const char**);
};

#endif
