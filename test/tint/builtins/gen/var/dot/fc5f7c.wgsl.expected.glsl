#version 310 es

int tint_int_dot(ivec2 a, ivec2 b) {
  return a[0]*b[0] + a[1]*b[1];
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void dot_fc5f7c() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  int res = tint_int_dot(arg_0, arg_1);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  dot_fc5f7c();
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

int tint_int_dot(ivec2 a, ivec2 b) {
  return a[0]*b[0] + a[1]*b[1];
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void dot_fc5f7c() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  int res = tint_int_dot(arg_0, arg_1);
  prevent_dce.inner = res;
}

void fragment_main() {
  dot_fc5f7c();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

int tint_int_dot(ivec2 a, ivec2 b) {
  return a[0]*b[0] + a[1]*b[1];
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void dot_fc5f7c() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  int res = tint_int_dot(arg_0, arg_1);
  prevent_dce.inner = res;
}

void compute_main() {
  dot_fc5f7c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
