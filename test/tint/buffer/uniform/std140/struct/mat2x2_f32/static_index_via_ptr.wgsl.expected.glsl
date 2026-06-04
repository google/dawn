#version 310 es


struct Inner {
  mat2 m;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[64];
} v;
mat2 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  vec2 v_3 = uintBitsToFloat(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  return mat2(v_3, uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u)))));
}
Inner v_6(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_7(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_6((start_byte_offset + (v_9 * 64u)));
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  return a;
}
Outer v_10(uint start_byte_offset) {
  return Outer(v_7(start_byte_offset));
}
Outer[4] v_11(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))))));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = v_10((start_byte_offset + (v_13 * 256u)));
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v_11(0u);
  Outer l_a_3 = v_10(768u);
  Inner l_a_3_a[4] = v_7(768u);
  Inner l_a_3_a_2 = v_6(896u);
  mat2 l_a_3_a_2_m = v_1(896u);
  vec2 l_a_3_a_2_m_1 = uintBitsToFloat(v.inner[56u].zw);
  uvec4 v_14 = v.inner[56u];
  float l_a_3_a_2_m_1_0 = uintBitsToFloat(v_14.z);
}
