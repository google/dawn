#version 310 es

int tint_int_dot(ivec4 a, ivec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

int tint_dot4_i8_packed(uint a, uint b) {
  ivec4 a_i8 = (ivec4((uvec4(a) << uvec4(24u, 16u, 8u, 0u))) >> uvec4(24u));
  ivec4 b_i8 = (ivec4((uvec4(b) << uvec4(24u, 16u, 8u, 0u))) >> uvec4(24u));
  return tint_int_dot(a_i8, b_i8);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  int res = tint_dot4_i8_packed(arg_0, arg_1);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  dot4I8Packed_881e62();
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

int tint_int_dot(ivec4 a, ivec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

int tint_dot4_i8_packed(uint a, uint b) {
  ivec4 a_i8 = (ivec4((uvec4(a) << uvec4(24u, 16u, 8u, 0u))) >> uvec4(24u));
  ivec4 b_i8 = (ivec4((uvec4(b) << uvec4(24u, 16u, 8u, 0u))) >> uvec4(24u));
  return tint_int_dot(a_i8, b_i8);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  int res = tint_dot4_i8_packed(arg_0, arg_1);
  prevent_dce.inner = res;
}

void fragment_main() {
  dot4I8Packed_881e62();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

int tint_int_dot(ivec4 a, ivec4 b) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

int tint_dot4_i8_packed(uint a, uint b) {
  ivec4 a_i8 = (ivec4((uvec4(a) << uvec4(24u, 16u, 8u, 0u))) >> uvec4(24u));
  ivec4 b_i8 = (ivec4((uvec4(b) << uvec4(24u, 16u, 8u, 0u))) >> uvec4(24u));
  return tint_int_dot(a_i8, b_i8);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void dot4I8Packed_881e62() {
  uint arg_0 = 1u;
  uint arg_1 = 1u;
  int res = tint_dot4_i8_packed(arg_0, arg_1);
  prevent_dce.inner = res;
}

void compute_main() {
  dot4I8Packed_881e62();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
