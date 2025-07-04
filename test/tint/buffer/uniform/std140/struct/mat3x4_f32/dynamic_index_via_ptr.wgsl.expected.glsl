#version 310 es


struct Inner {
  mat3x4 m;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_1_ubo {
  Outer inner[4];
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_2 = min(uint(i()), 3u);
  uint v_3 = min(uint(i()), 3u);
  uint v_4 = min(uint(i()), 2u);
  Outer l_a[4] = v.inner;
  Outer l_a_i = v.inner[v_2];
  Inner l_a_i_a[4] = v.inner[v_2].a;
  Inner l_a_i_a_i = v.inner[v_2].a[v_3];
  mat3x4 l_a_i_a_i_m = v.inner[v_2].a[v_3].m;
  vec4 l_a_i_a_i_m_i = v.inner[v_2].a[v_3].m[v_4];
  float l_a_i_a_i_m_i_i = v.inner[v_2].a[v_3].m[v_4][min(uint(i()), 3u)];
}
