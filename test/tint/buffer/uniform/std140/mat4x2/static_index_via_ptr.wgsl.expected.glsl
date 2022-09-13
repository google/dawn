#version 310 es

struct Inner {
  mat4x2 m;
};

struct Inner_std140 {
  vec2 m_0;
  vec2 m_1;
  vec2 m_2;
  vec2 m_3;
};

struct Outer {
  Inner a[4];
};

struct Outer_std140 {
  Inner_std140 a[4];
};

layout(binding = 0, std140) uniform a_block_ubo {
  Outer_std140 inner[4];
} a;

Inner conv_Inner(Inner_std140 val) {
  Inner tint_symbol = Inner(mat4x2(val.m_0, val.m_1, val.m_2, val.m_3));
  return tint_symbol;
}

Inner[4] conv_arr_4_Inner(Inner_std140 val[4]) {
  Inner arr[4] = Inner[4](Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_Inner(val[i]);
    }
  }
  return arr;
}

Outer conv_Outer(Outer_std140 val) {
  Outer tint_symbol_1 = Outer(conv_arr_4_Inner(val.a));
  return tint_symbol_1;
}

Outer[4] conv_arr_4_Outer(Outer_std140 val[4]) {
  Outer arr[4] = Outer[4](Outer(Inner[4](Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)))), Outer(Inner[4](Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)))), Outer(Inner[4](Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)))), Outer(Inner[4](Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)), Inner(mat4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f)))));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_Outer(val[i]);
    }
  }
  return arr;
}

mat4x2 load_a_3_a_2_m() {
  return mat4x2(a.inner[3u].a[2u].m_0, a.inner[3u].a[2u].m_1, a.inner[3u].a[2u].m_2, a.inner[3u].a[2u].m_3);
}

void f() {
  int I = 1;
  Outer p_a[4] = conv_arr_4_Outer(a.inner);
  Outer p_a_3 = conv_Outer(a.inner[3u]);
  Inner p_a_3_a[4] = conv_arr_4_Inner(a.inner[3u].a);
  Inner p_a_3_a_2 = conv_Inner(a.inner[3u].a[2u]);
  mat4x2 p_a_3_a_2_m = load_a_3_a_2_m();
  vec2 p_a_3_a_2_m_1 = a.inner[3u].a[2u].m_1;
  Outer l_a[4] = conv_arr_4_Outer(a.inner);
  Outer l_a_3 = conv_Outer(a.inner[3u]);
  Inner l_a_3_a[4] = conv_arr_4_Inner(a.inner[3u].a);
  Inner l_a_3_a_2 = conv_Inner(a.inner[3u].a[2u]);
  mat4x2 l_a_3_a_2_m = load_a_3_a_2_m();
  vec2 l_a_3_a_2_m_1 = a.inner[3u].a[2u].m_1;
  float l_a_3_a_2_m_1_0 = a.inner[3u].a[2u].m_1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
