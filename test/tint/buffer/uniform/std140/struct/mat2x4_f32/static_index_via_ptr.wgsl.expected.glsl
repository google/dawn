#version 310 es


struct Inner {
  mat2x4 m;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[64];
} v;
mat2x4 v_1(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
Inner v_2(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_3(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))));
  {
    uint idx = 0u;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 4u)) {
        break;
      }
      a[v_4] = v_2((start_byte_offset + (v_4 * 64u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
Outer v_5(uint start_byte_offset) {
  return Outer(v_3(start_byte_offset));
}
Outer[4] v_6(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))))), Outer(Inner[4](Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))))), Outer(Inner[4](Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))))), Outer(Inner[4](Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))), Inner(mat2x4(vec4(0.0f), vec4(0.0f))))));
  {
    uint idx = 0u;
    while(true) {
      uint v_7 = idx;
      if ((v_7 >= 4u)) {
        break;
      }
      a[v_7] = v_5((start_byte_offset + (v_7 * 256u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v_6(0u);
  Outer l_a_3 = v_5(768u);
  Inner l_a_3_a[4] = v_3(768u);
  Inner l_a_3_a_2 = v_2(896u);
  mat2x4 l_a_3_a_2_m = v_1(896u);
  vec4 l_a_3_a_2_m_1 = uintBitsToFloat(v.inner[57u]);
  uvec4 v_8 = v.inner[57u];
  float l_a_3_a_2_m_1_0 = uintBitsToFloat(v_8.x);
}
