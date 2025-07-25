#version 310 es

int i(int x) {
  return int((~(uint(x)) + 1u));
}
ivec4 vi(ivec4 x) {
  return ivec4((~(uvec4(x)) + uvec4(1u)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
