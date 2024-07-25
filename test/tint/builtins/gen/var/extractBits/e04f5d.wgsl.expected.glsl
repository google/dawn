#version 310 es
precision highp float;
precision highp int;

ivec3 tint_extract_bits(ivec3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec3 inner;
} prevent_dce;

ivec3 extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = extractBits_e04f5d();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

ivec3 tint_extract_bits(ivec3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  ivec3 inner;
} prevent_dce;

ivec3 extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = extractBits_e04f5d();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es

ivec3 tint_extract_bits(ivec3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

layout(location = 0) flat out ivec3 prevent_dce_1;
ivec3 extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 res = tint_extract_bits(arg_0, arg_1, arg_2);
  return res;
}

struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), ivec3(0, 0, 0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = extractBits_e04f5d();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
