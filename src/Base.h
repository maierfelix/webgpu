#define NAPI_EXPERIMENTAL
#include <napi.h>

#include <GLFW/glfw3.h>

#include <dawn/dawn.h>
#include <dawn/dawn_wsi.h>
#include <dawn/dawncpp.h>
#include <dawn_native/DawnNative.h>
#include <dawn_wire/WireClient.h>
#include <dawn_wire/WireServer.h>

#include <algorithm>
