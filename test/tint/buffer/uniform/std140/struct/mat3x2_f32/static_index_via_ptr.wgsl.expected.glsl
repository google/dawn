#version 310 es


struct Inner {
  mat3x2 m;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[64];
} v;
mat3x2 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  vec2 v_3 = uintBitsToFloat(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  vec2 v_6 = uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u))));
  uint v_7 = (16u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  return mat3x2(v_3, v_6, uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2((((v_7 & 15u) >> 2u) == 2u)))));
}
Inner v_9(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_10(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_9((start_byte_offset + (v_12 * 64u)));
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
  return a;
}
Outer v_13(uint start_byte_offset) {
  return Outer(v_10(start_byte_offset));
}
Outer[4] v_14(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f))))));
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      a[v_16] = v_13((start_byte_offset + (v_16 * 256u)));
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v_14(0u);
  Outer l_a_3 = v_13(768u);
  Inner l_a_3_a[4] = v_10(768u);
  Inner l_a_3_a_2 = v_9(896u);
  mat3x2 l_a_3_a_2_m = v_1(896u);
  vec2 l_a_3_a_2_m_1 = uintBitsToFloat(v.inner[56u].zw);
  uvec4 v_17 = v.inner[56u];
  float l_a_3_a_2_m_1_0 = uintBitsToFloat(v_17.z);
}
