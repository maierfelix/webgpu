#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 uv;
layout (location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 0) buffer PixelBuffer {
  vec4 pixels[];
} pixelBuffer;

layout(set = 0, binding = 1) uniform ScreenDimension {
  vec2 resolution;
};

void main() {
  const ivec2 bufferCoord = ivec2(floor(uv * resolution));
  const vec2 fragCoord = (uv * resolution);
  const uint pixelIndex = bufferCoord.y * uint(resolution.x) + bufferCoord.x;

  vec4 pixelColor = pixelBuffer.pixels[pixelIndex];
  outColor = pixelColor;
}
