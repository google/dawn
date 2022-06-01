#version 310 es

ivec3 tint_insert_bits(ivec3 v, ivec3 n, uint offset, uint count) {
  uint s = min(offset, 32u);
  uint e = min(32u, (s + count));
  return bitfieldInsert(v, n, int(s), int((e - s)));
}

void f_1() {
  ivec3 v = ivec3(0);
  ivec3 n = ivec3(0);
  uint offset_1 = 0u;
  uint count = 0u;
  ivec3 x_16 = tint_insert_bits(v, n, offset_1, count);
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
