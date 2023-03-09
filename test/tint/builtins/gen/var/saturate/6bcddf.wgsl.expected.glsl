#version 310 es

vec3 tint_saturate(vec3 v) {
  return clamp(v, vec3(0.0f), vec3(1.0f));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

void saturate_6bcddf() {
  vec3 arg_0 = vec3(2.0f);
  vec3 res = tint_saturate(arg_0);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  saturate_6bcddf();
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

vec3 tint_saturate(vec3 v) {
  return clamp(v, vec3(0.0f), vec3(1.0f));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

void saturate_6bcddf() {
  vec3 arg_0 = vec3(2.0f);
  vec3 res = tint_saturate(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  saturate_6bcddf();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

vec3 tint_saturate(vec3 v) {
  return clamp(v, vec3(0.0f), vec3(1.0f));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

void saturate_6bcddf() {
  vec3 arg_0 = vec3(2.0f);
  vec3 res = tint_saturate(arg_0);
  prevent_dce.inner = res;
}

void compute_main() {
  saturate_6bcddf();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
