#version 310 es

layout(binding = 2, std430)
buffer _storage_block_1_ssbo {
  ivec2 inner;
} v;
void tint_symbol() {
  ivec2 vec = ivec2(0);
  {
    while(true) {
      if ((vec.y >= v.inner.y)) {
        break;
      }
      if ((vec.y >= 0)) {
        break;
      }
      {
      }
      continue;
    }
  }
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
