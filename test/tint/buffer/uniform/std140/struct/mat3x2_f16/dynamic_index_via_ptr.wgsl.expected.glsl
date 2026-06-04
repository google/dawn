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
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat3x2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  f16vec2 v_5 = tint_bitcast_to_16bit(v.inner[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  return f16mat3x2(v_3, v_5, tint_bitcast_to_16bit(v.inner[(v_6 / 16u)][((v_6 & 15u) >> 2u)]));
}
Inner v_7(uint start_byte_offset) {
  return Inner(v_2(start_byte_offset));
}
Inner[4] v_8(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))));
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
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))));
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
  uint v_17 = (min(uint(i()), 2u) * 4u);
  Outer l_a[4] = v_12(0u);
  Outer l_a_i = v_11(v_15);
  Inner l_a_i_a[4] = v_8(v_15);
  Inner l_a_i_a_i = v_7((v_15 + v_16));
  f16mat3x2 l_a_i_a_i_m = v_2((v_15 + v_16));
  uint v_18 = ((v_15 + v_16) + v_17);
  f16vec2 l_a_i_a_i_m_i = tint_bitcast_to_16bit(v.inner[(v_18 / 16u)][((v_18 & 15u) >> 2u)]);
  uint v_19 = (((v_15 + v_16) + v_17) + (min(uint(i()), 1u) * 2u));
  uvec4 v_20 = v.inner[(v_19 / 16u)];
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_16bit(v_20[((v_19 & 15u) >> 2u)])[mix(1u, 0u, ((v_19 % 4u) == 0u))];
}
