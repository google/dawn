#version 310 es

ivec3 tint_extract_bits(ivec3 v, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldExtract(v, int(s), int((e - s)));
}

void f_1() {
  ivec3 v = ivec3(0);
  uint offset_1 = 0u;
  uint count = 0u;
  ivec3 x_15 = tint_extract_bits(v, offset_1, count);
  return;
}

void f() {
  f_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
