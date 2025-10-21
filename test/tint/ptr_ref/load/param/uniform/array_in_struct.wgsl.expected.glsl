#version 310 es

layout(binding = 0, std140)
uniform S_block_1_ubo {
  uvec4 inner[4];
} v;
ivec4[4] v_1(uint start_byte_offset) {
  ivec4 a[4] = ivec4[4](ivec4(0), ivec4(0), ivec4(0), ivec4(0));
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      a[v_3] = ivec4(v.inner[((start_byte_offset + (v_3 * 16u)) / 16u)]);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  return a;
}
ivec4[4] func() {
  return v_1(0u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  ivec4 r[4] = func();
}
