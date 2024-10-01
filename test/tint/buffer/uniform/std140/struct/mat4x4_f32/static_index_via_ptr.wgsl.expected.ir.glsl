#version 310 es


struct Inner {
  mat4 m;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform tint_symbol_1_1_ubo {
  Outer tint_symbol[4];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v.tint_symbol;
  Outer l_a_3 = v.tint_symbol[3];
  Inner l_a_3_a[4] = v.tint_symbol[3].a;
  Inner l_a_3_a_2 = v.tint_symbol[3].a[2];
  mat4 l_a_3_a_2_m = v.tint_symbol[3].a[2].m;
  vec4 l_a_3_a_2_m_1 = v.tint_symbol[3].a[2].m[1];
  float l_a_3_a_2_m_1_0 = v.tint_symbol[3].a[2].m[1].x;
}
