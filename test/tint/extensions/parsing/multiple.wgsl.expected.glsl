#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision highp float;

layout(location = 0) out vec4 value;
vec4 tint_symbol() {
  return vec4(0.10000000149011611938f, 0.20000000298023223877f, 0.30000001192092895508f, 0.40000000596046447754f);
}

void main() {
  vec4 inner_result = tint_symbol();
  value = inner_result;
  return;
}
