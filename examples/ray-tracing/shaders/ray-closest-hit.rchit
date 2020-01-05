#version 460
#extension GL_NV_ray_tracing : require
#pragma shader_stage(closest)

layout(location = 0) rayPayloadInNV vec3 hitValue;

hitAttributeNV vec3 attribs;

void main() {
  const vec3 bary = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
  hitValue = bary;
}
