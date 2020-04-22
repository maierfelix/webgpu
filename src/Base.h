#define NAPI_EXPERIMENTAL
#include <napi.h>

#include <GLFW/glfw3.h>

#include <dawn/dawn_proc.h>
#include <dawn/webgpu_cpp.h>
#include <dawn_native/DawnNative.h>
#include <dawn_wire/WireClient.h>
#include <dawn_wire/WireServer.h>

#include <algorithm>

#include "Utils.h"
