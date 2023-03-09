#version 310 es

ivec4 tint_insert_bits(ivec4 v, ivec4 n, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldInsert(v, n, int(s), int((e - s)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void insertBits_d86978() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  ivec4 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  insertBits_d86978();
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

ivec4 tint_insert_bits(ivec4 v, ivec4 n, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldInsert(v, n, int(s), int((e - s)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void insertBits_d86978() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  ivec4 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  prevent_dce.inner = res;
}

void fragment_main() {
  insertBits_d86978();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

ivec4 tint_insert_bits(ivec4 v, ivec4 n, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldInsert(v, n, int(s), int((e - s)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec4 inner;
} prevent_dce;

void insertBits_d86978() {
  ivec4 arg_0 = ivec4(1);
  ivec4 arg_1 = ivec4(1);
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  ivec4 res = tint_insert_bits(arg_0, arg_1, arg_2, arg_3);
  prevent_dce.inner = res;
}

void compute_main() {
  insertBits_d86978();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
