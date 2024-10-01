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
int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int v_1 = i();
  int v_2 = i();
  int v_3 = i();
  Outer l_a[4] = v.tint_symbol;
  Outer l_a_i = v.tint_symbol[v_1];
  Inner l_a_i_a[4] = v.tint_symbol[v_1].a;
  Inner l_a_i_a_i = v.tint_symbol[v_1].a[v_2];
  mat4 l_a_i_a_i_m = v.tint_symbol[v_1].a[v_2].m;
  vec4 l_a_i_a_i_m_i = v.tint_symbol[v_1].a[v_2].m[v_3];
  float l_a_i_a_i_m_i_i = v.tint_symbol[v_1].a[v_2].m[v_3][i()];
}
