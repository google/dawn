#version 310 es


struct Inner_std140 {
  vec2 m_col0;
  vec2 m_col1;
  vec2 m_col2;
  vec2 m_col3;
  uint tint_pad_0;
  uint tint_pad_1;
  uint tint_pad_2;
  uint tint_pad_3;
  uint tint_pad_4;
  uint tint_pad_5;
  uint tint_pad_6;
  uint tint_pad_7;
};

struct Outer_std140 {
  Inner_std140 a[4];
};

struct Inner {
  mat4x2 m;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_std140_1_ubo {
  Outer_std140 inner[4];
} v;
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
Inner tint_convert_Inner(Inner_std140 tint_input) {
  return Inner(mat4x2(tint_input.m_col0, tint_input.m_col1, tint_input.m_col2, tint_input.m_col3));
}
Outer tint_convert_Outer(Outer_std140 tint_input) {
  Inner v_2[4] = Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      v_2[v_4] = tint_convert_Inner(tint_input.a[v_4]);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  return Outer(v_2);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_5 = min(uint(i()), 3u);
  uint v_6 = min(uint(i()), 3u);
  mat4x2 v_7 = mat4x2(v.inner[v_5].a[v_6].m_col0, v.inner[v_5].a[v_6].m_col1, v.inner[v_5].a[v_6].m_col2, v.inner[v_5].a[v_6].m_col3);
  vec2 v_8 = v_7[min(uint(i()), 3u)];
  Outer_std140 v_9[4] = v.inner;
  Outer v_10[4] = Outer[4](Outer(Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))))));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      v_10[v_12] = tint_convert_Outer(v_9[v_12]);
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  Outer l_a[4] = v_10;
  Outer l_a_i = tint_convert_Outer(v.inner[v_5]);
  Inner_std140 v_13[4] = v.inner[v_5].a;
  Inner v_14[4] = Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))));
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      v_14[v_16] = tint_convert_Inner(v_13[v_16]);
      {
        v_15 = (v_16 + 1u);
      }
      continue;
    }
  }
  Inner l_a_i_a[4] = v_14;
  Inner l_a_i_a_i = tint_convert_Inner(v.inner[v_5].a[v_6]);
  mat4x2 l_a_i_a_i_m = v_7;
  vec2 l_a_i_a_i_m_i = v_8;
  float l_a_i_a_i_m_i_i = v_8[min(uint(i()), 1u)];
}
