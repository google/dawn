#version 310 es

vec4 tint_saturate(vec4 v) {
  return clamp(v, vec4(0.0f), vec4(1.0f));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void saturate_a5b571() {
  vec4 arg_0 = vec4(2.0f);
  vec4 res = tint_saturate(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  saturate_a5b571();
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
precision highp float;

vec4 tint_saturate(vec4 v) {
  return clamp(v, vec4(0.0f), vec4(1.0f));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void saturate_a5b571() {
  vec4 arg_0 = vec4(2.0f);
  vec4 res = tint_saturate(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  saturate_a5b571();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec4 tint_saturate(vec4 v) {
  return clamp(v, vec4(0.0f), vec4(1.0f));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void saturate_a5b571() {
  vec4 arg_0 = vec4(2.0f);
  vec4 res = tint_saturate(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  saturate_a5b571();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
