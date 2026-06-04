#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  f16mat3x4 m;
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
f16vec2 tint_bitcast_to_16bit_1(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat3x4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_16bit(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  f16vec4 v_7 = tint_bitcast_to_16bit(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u))));
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v.inner[(v_8 / 16u)];
  return f16mat3x4(v_4, v_7, tint_bitcast_to_16bit(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u)))));
}
Inner v_10(uint start_byte_offset) {
  return Inner(v_2(start_byte_offset));
}
Inner[4] v_11(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = v_10((start_byte_offset + (v_13 * 64u)));
      {
        v_12 = (v_13 + 1u);
      }
    }
  }
  return a;
}
Outer v_14(uint start_byte_offset) {
  return Outer(v_11(start_byte_offset));
}
Outer[4] v_15(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))));
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      a[v_17] = v_14((start_byte_offset + (v_17 * 256u)));
      {
        v_16 = (v_17 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_18 = (min(uint(i()), 3u) * 256u);
  uint v_19 = (min(uint(i()), 3u) * 64u);
  uint v_20 = (min(uint(i()), 2u) * 8u);
  Outer l_a[4] = v_15(0u);
  Outer l_a_i = v_14(v_18);
  Inner l_a_i_a[4] = v_11(v_18);
  Inner l_a_i_a_i = v_10((v_18 + v_19));
  f16mat3x4 l_a_i_a_i_m = v_2((v_18 + v_19));
  uint v_21 = ((v_18 + v_19) + v_20);
  uvec4 v_22 = v.inner[(v_21 / 16u)];
  f16vec4 l_a_i_a_i_m_i = tint_bitcast_to_16bit(mix(v_22.xy, v_22.zw, bvec2((((v_21 & 15u) >> 2u) == 2u))));
  uint v_23 = (((v_18 + v_19) + v_20) + (min(uint(i()), 3u) * 2u));
  uvec4 v_24 = v.inner[(v_23 / 16u)];
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_16bit_1(v_24[((v_23 & 15u) >> 2u)])[mix(1u, 0u, ((v_23 % 4u) == 0u))];
}
