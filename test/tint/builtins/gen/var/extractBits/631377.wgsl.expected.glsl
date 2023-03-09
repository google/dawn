#version 310 es

uvec4 tint_extract_bits(uvec4 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

void extractBits_631377() {
  uvec4 arg_0 = uvec4(1u);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uvec4 res = tint_extract_bits(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  extractBits_631377();
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

uvec4 tint_extract_bits(uvec4 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

void extractBits_631377() {
  uvec4 arg_0 = uvec4(1u);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uvec4 res = tint_extract_bits(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

void fragment_main() {
  extractBits_631377();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uvec4 tint_extract_bits(uvec4 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uvec4 inner;
} prevent_dce;

void extractBits_631377() {
  uvec4 arg_0 = uvec4(1u);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uvec4 res = tint_extract_bits(arg_0, arg_1, arg_2);
  prevent_dce.inner = res;
}

void compute_main() {
  extractBits_631377();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
