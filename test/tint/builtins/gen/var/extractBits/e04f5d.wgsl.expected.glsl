#version 310 es

ivec3 tint_extract_bits(ivec3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

void extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 res = tint_extract_bits(arg_0, arg_1, arg_2);
}

vec4 vertex_main() {
  extractBits_e04f5d();
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
precision mediump float;

ivec3 tint_extract_bits(ivec3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

void extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 res = tint_extract_bits(arg_0, arg_1, arg_2);
}

void fragment_main() {
  extractBits_e04f5d();
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

void extractBits_e04f5d() {
  ivec3 arg_0 = ivec3(1);
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  ivec3 res = tint_extract_bits(arg_0, arg_1, arg_2);
}

void compute_main() {
  extractBits_e04f5d();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
