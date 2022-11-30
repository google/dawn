#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct Inner {
  f16mat2x4 m;
  uint pad;
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
};

struct Inner_std140 {
  f16vec4 m_0;
  f16vec4 m_1;
  uint pad;
  uint pad_1;
  uint pad_2;
  uint pad_3;
  uint pad_4;
  uint pad_5;
  uint pad_6;
  uint pad_7;
  uint pad_8;
  uint pad_9;
  uint pad_10;
  uint pad_11;
};

struct Outer {
  Inner a[4];
};

struct Outer_std140 {
  Inner_std140 a[4];
};

layout(binding = 0, std140) uniform a_block_std140_ubo {
  Outer_std140 inner[4];
} a;

Inner conv_Inner(Inner_std140 val) {
  return Inner(f16mat2x4(val.m_0, val.m_1), val.pad, val.pad_1, val.pad_2, val.pad_3, val.pad_4, val.pad_5, val.pad_6, val.pad_7, val.pad_8, val.pad_9, val.pad_10, val.pad_11);
}

Inner[4] conv_arr4_Inner(Inner_std140 val[4]) {
  Inner arr[4] = Inner[4](Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_Inner(val[i]);
    }
  }
  return arr;
}

Outer conv_Outer(Outer_std140 val) {
  return Outer(conv_arr4_Inner(val.a));
}

Outer[4] conv_arr4_Outer(Outer_std140 val[4]) {
  Outer arr[4] = Outer[4](Outer(Inner[4](Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u))), Outer(Inner[4](Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u))), Outer(Inner[4](Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u))), Outer(Inner[4](Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u), Inner(f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u))));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_Outer(val[i]);
    }
  }
  return arr;
}

f16mat2x4 load_a_inner_3_a_2_m() {
  return f16mat2x4(a.inner[3u].a[2u].m_0, a.inner[3u].a[2u].m_1);
}

void f() {
  Outer p_a[4] = conv_arr4_Outer(a.inner);
  Outer p_a_3 = conv_Outer(a.inner[3u]);
  Inner p_a_3_a[4] = conv_arr4_Inner(a.inner[3u].a);
  Inner p_a_3_a_2 = conv_Inner(a.inner[3u].a[2u]);
  f16mat2x4 p_a_3_a_2_m = load_a_inner_3_a_2_m();
  f16vec4 p_a_3_a_2_m_1 = a.inner[3u].a[2u].m_1;
  Outer l_a[4] = conv_arr4_Outer(a.inner);
  Outer l_a_3 = conv_Outer(a.inner[3u]);
  Inner l_a_3_a[4] = conv_arr4_Inner(a.inner[3u].a);
  Inner l_a_3_a_2 = conv_Inner(a.inner[3u].a[2u]);
  f16mat2x4 l_a_3_a_2_m = load_a_inner_3_a_2_m();
  f16vec4 l_a_3_a_2_m_1 = a.inner[3u].a[2u].m_1;
  float16_t l_a_3_a_2_m_1_0 = a.inner[3u].a[2u].m_1[0u];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
