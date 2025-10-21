#version 310 es


struct Inner {
  mat2x3 m;
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
mat2x3 v_2(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
Inner v_3(uint start_byte_offset) {
  return Inner(v_2(start_byte_offset));
}
Inner[4] v_4(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a[v_6] = v_3((start_byte_offset + (v_6 * 64u)));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  return a;
}
Outer v_7(uint start_byte_offset) {
  return Outer(v_4(start_byte_offset));
}
Outer[4] v_8(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))))), Outer(Inner[4](Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))))), Outer(Inner[4](Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))))), Outer(Inner[4](Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))), Inner(mat2x3(vec3(0.0f), vec3(0.0f))))));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_7((start_byte_offset + (v_10 * 256u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_11 = (256u * min(uint(i()), 3u));
  uint v_12 = (64u * min(uint(i()), 3u));
  uint v_13 = (16u * min(uint(i()), 1u));
  Outer l_a[4] = v_8(0u);
  Outer l_a_i = v_7(v_11);
  Inner l_a_i_a[4] = v_4(v_11);
  Inner l_a_i_a_i = v_3((v_11 + v_12));
  mat2x3 l_a_i_a_i_m = v_2((v_11 + v_12));
  vec3 l_a_i_a_i_m_i = uintBitsToFloat(v.inner[(((v_11 + v_12) + v_13) / 16u)].xyz);
  uint v_14 = (((v_11 + v_12) + v_13) + (min(uint(i()), 2u) * 4u));
  uvec4 v_15 = v.inner[(v_14 / 16u)];
  float l_a_i_a_i_m_i_i = uintBitsToFloat(v_15[((v_14 % 16u) / 4u)]);
}
