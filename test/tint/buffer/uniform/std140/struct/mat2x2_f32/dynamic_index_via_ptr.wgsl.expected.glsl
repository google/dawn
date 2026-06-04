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
int counter = 0;
int i() {
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
mat2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  return mat2(v_4, uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))));
}
Inner v_7(uint start_byte_offset) {
  return Inner(v_2(start_byte_offset));
}
Inner[4] v_8(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_7((start_byte_offset + (v_10 * 64u)));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  return a;
}
Outer v_11(uint start_byte_offset) {
  return Outer(v_8(start_byte_offset));
}
Outer[4] v_12(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))))), Outer(Inner[4](Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))), Inner(mat2(vec2(0.0f), vec2(0.0f))))));
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = v_11((start_byte_offset + (v_14 * 256u)));
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_15 = (min(uint(i()), 3u) * 256u);
  uint v_16 = (min(uint(i()), 3u) * 64u);
  uint v_17 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_12(0u);
  Outer l_a_i = v_11(v_15);
  Inner l_a_i_a[4] = v_8(v_15);
  Inner l_a_i_a_i = v_7((v_15 + v_16));
  mat2 l_a_i_a_i_m = v_2((v_15 + v_16));
  uint v_18 = ((v_15 + v_16) + v_17);
  uvec4 v_19 = v.inner[(v_18 / 16u)];
  vec2 l_a_i_a_i_m_i = uintBitsToFloat(mix(v_19.xy, v_19.zw, bvec2((((v_18 & 15u) >> 2u) == 2u))));
  uint v_20 = (((v_15 + v_16) + v_17) + (min(uint(i()), 1u) * 4u));
  uvec4 v_21 = v.inner[(v_20 / 16u)];
  float l_a_i_a_i_m_i_i = uintBitsToFloat(v_21[((v_20 & 15u) >> 2u)]);
}
