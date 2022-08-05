#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec4 tint_atanh(f16vec4 x) {
  return mix(atanh(x), f16vec4(0.0hf), greaterThanEqual(x, f16vec4(1.0hf)));
}

void atanh_e3b450() {
  f16vec4 res = tint_atanh(f16vec4(0.0hf));
}

vec4 vertex_main() {
  atanh_e3b450();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require
precision mediump float;

f16vec4 tint_atanh(f16vec4 x) {
  return mix(atanh(x), f16vec4(0.0hf), greaterThanEqual(x, f16vec4(1.0hf)));
}

void atanh_e3b450() {
  f16vec4 res = tint_atanh(f16vec4(0.0hf));
}

void fragment_main() {
  atanh_e3b450();
}

void main() {
  fragment_main();
  return;
}
#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

f16vec4 tint_atanh(f16vec4 x) {
  return mix(atanh(x), f16vec4(0.0hf), greaterThanEqual(x, f16vec4(1.0hf)));
}

void atanh_e3b450() {
  f16vec4 res = tint_atanh(f16vec4(0.0hf));
}

void compute_main() {
  atanh_e3b450();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
