#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  f16mat2 m;
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
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  return f16mat2(v_2, tint_bitcast_to_16bit(v.inner[(v_3 / 16u)][((v_3 & 15u) >> 2u)]));
}
Inner v_4(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_5(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))));
  {
    uint idx = 0u;
    while(true) {
      uint v_6 = idx;
      if ((v_6 >= 4u)) {
        break;
      }
      a[v_6] = v_4((start_byte_offset + (v_6 * 64u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
Outer v_7(uint start_byte_offset) {
  return Outer(v_5(start_byte_offset));
}
Outer[4] v_8(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))));
  {
    uint idx = 0u;
    while(true) {
      uint v_9 = idx;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_7((start_byte_offset + (v_9 * 256u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_10 = (min(uint(i()), 3u) * 256u);
  uint v_11 = (min(uint(i()), 3u) * 64u);
  uint v_12 = (min(uint(i()), 1u) * 4u);
  Outer l_a[4] = v_8(0u);
  Outer l_a_i = v_7(v_10);
  Inner l_a_i_a[4] = v_5(v_10);
  Inner l_a_i_a_i = v_4((v_10 + v_11));
  f16mat2 l_a_i_a_i_m = v_1((v_10 + v_11));
  uint v_13 = ((v_10 + v_11) + v_12);
  f16vec2 l_a_i_a_i_m_i = tint_bitcast_to_16bit(v.inner[(v_13 / 16u)][((v_13 & 15u) >> 2u)]);
  uint v_14 = (((v_10 + v_11) + v_12) + (min(uint(i()), 1u) * 2u));
  uvec4 v_15 = v.inner[(v_14 / 16u)];
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_16bit(v_15[((v_14 & 15u) >> 2u)])[mix(1u, 0u, ((v_14 % 4u) == 0u))];
}
