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
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v.inner;
  Outer l_a_3 = v.inner[3];
  Inner l_a_3_a[4] = v.inner[3].a;
  Inner l_a_3_a_2 = v.inner[3].a[2];
  mat3x4 l_a_3_a_2_m = v.inner[3].a[2].m;
  vec4 l_a_3_a_2_m_1 = v.inner[3].a[2].m[1];
  float l_a_3_a_2_m_1_0 = v.inner[3].a[2].m[1].x;
}
