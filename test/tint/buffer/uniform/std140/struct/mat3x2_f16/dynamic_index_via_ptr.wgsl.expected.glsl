#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  f16mat3x2 m;
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
f16mat3x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  f16vec2 v_4 = tint_bitcast_to_16bit(v.inner[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  return f16mat3x2(v_2, v_4, tint_bitcast_to_16bit(v.inner[(v_5 / 16u)][((v_5 & 15u) >> 2u)]));
}
Inner v_6(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_7(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))));
  {
    uint idx = 0u;
    while(true) {
      uint v_8 = idx;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_6((start_byte_offset + (v_8 * 64u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
Outer v_9(uint start_byte_offset) {
  return Outer(v_7(start_byte_offset));
}
Outer[4] v_10(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))));
  {
    uint idx = 0u;
    while(true) {
      uint v_11 = idx;
      if ((v_11 >= 4u)) {
        break;
      }
      a[v_11] = v_9((start_byte_offset + (v_11 * 256u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_12 = (min(uint(i()), 3u) * 256u);
  uint v_13 = (min(uint(i()), 3u) * 64u);
  uint v_14 = (min(uint(i()), 2u) * 4u);
  Outer l_a[4] = v_10(0u);
  Outer l_a_i = v_9(v_12);
  Inner l_a_i_a[4] = v_7(v_12);
  Inner l_a_i_a_i = v_6((v_12 + v_13));
  f16mat3x2 l_a_i_a_i_m = v_1((v_12 + v_13));
  uint v_15 = ((v_12 + v_13) + v_14);
  f16vec2 l_a_i_a_i_m_i = tint_bitcast_to_16bit(v.inner[(v_15 / 16u)][((v_15 & 15u) >> 2u)]);
  uint v_16 = (((v_12 + v_13) + v_14) + (min(uint(i()), 1u) * 2u));
  uvec4 v_17 = v.inner[(v_16 / 16u)];
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_16bit(v_17[((v_16 & 15u) >> 2u)])[mix(1u, 0u, ((v_16 % 4u) == 0u))];
}
