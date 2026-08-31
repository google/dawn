#version 310 es


struct Inner {
  mat4x2 m;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[64];
} v;
int counter = 0;
int i() {
  counter = int((uint(counter) + 1u));
  return counter;
}
mat4x2 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  vec2 v_3 = uintBitsToFloat(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  vec2 v_6 = uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u))));
  uint v_7 = (16u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  vec2 v_9 = uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2((((v_7 & 15u) >> 2u) == 2u))));
  uint v_10 = (24u + start_byte_offset);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  return mat4x2(v_3, v_6, v_9, uintBitsToFloat(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u)))));
}
Inner v_12(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_13(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))));
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a[v_15] = v_12((start_byte_offset + (v_15 * 64u)));
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  return a;
}
Outer v_16(uint start_byte_offset) {
  return Outer(v_13(start_byte_offset));
}
Outer[4] v_17(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))), Inner(mat4x2(vec2(0.0f), vec2(0.0f), vec2(0.0f), vec2(0.0f))))));
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      a[v_19] = v_16((start_byte_offset + (v_19 * 256u)));
      {
        v_18 = (v_19 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_20 = (min(uint(i()), 3u) * 256u);
  uint v_21 = (min(uint(i()), 3u) * 64u);
  uint v_22 = (min(uint(i()), 3u) * 8u);
  Outer l_a[4] = v_17(0u);
  Outer l_a_i = v_16(v_20);
  Inner l_a_i_a[4] = v_13(v_20);
  Inner l_a_i_a_i = v_12((v_20 + v_21));
  mat4x2 l_a_i_a_i_m = v_1((v_20 + v_21));
  uint v_23 = ((v_20 + v_21) + v_22);
  uvec4 v_24 = v.inner[(v_23 / 16u)];
  vec2 l_a_i_a_i_m_i = uintBitsToFloat(mix(v_24.xy, v_24.zw, bvec2((((v_23 & 15u) >> 2u) == 2u))));
  uint v_25 = (((v_20 + v_21) + v_22) + (min(uint(i()), 1u) * 4u));
  uvec4 v_26 = v.inner[(v_25 / 16u)];
  float l_a_i_a_i_m_i_i = uintBitsToFloat(v_26[((v_25 & 15u) >> 2u)]);
}
