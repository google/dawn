#version 310 es

struct Inner {
  mat2x3 m;
  uint pad;
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  uint pad_7;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140) uniform a_block_ubo {
  Outer inner[4];
} a;

int counter = 0;
int i() {
  counter = (counter + 1);
  return counter;
}

void f() {
  int tint_symbol = i();
  int p_a_i_save = tint_symbol;
  int tint_symbol_1 = i();
  int p_a_i_a_i_save = tint_symbol_1;
  int tint_symbol_2 = i();
  int p_a_i_a_i_m_i_save = tint_symbol_2;
  Outer l_a[4] = a.inner;
  Outer l_a_i = a.inner[p_a_i_save];
  Inner l_a_i_a[4] = a.inner[p_a_i_save].a;
  Inner l_a_i_a_i = a.inner[p_a_i_save].a[p_a_i_a_i_save];
  mat2x3 l_a_i_a_i_m = a.inner[p_a_i_save].a[p_a_i_a_i_save].m;
  vec3 l_a_i_a_i_m_i = a.inner[p_a_i_save].a[p_a_i_a_i_save].m[p_a_i_a_i_m_i_save];
  int tint_symbol_3 = i();
  float l_a_i_a_i_m_i_i = a.inner[p_a_i_save].a[p_a_i_a_i_save].m[p_a_i_a_i_m_i_save][tint_symbol_3];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
