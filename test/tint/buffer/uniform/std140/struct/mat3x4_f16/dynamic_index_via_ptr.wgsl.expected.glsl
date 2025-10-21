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
f16vec2 tint_bitcast_to_f16_1(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat3x4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_f16(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_6 = tint_bitcast_to_f16(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_7 = v.inner[((16u + start_byte_offset) / 16u)];
  return f16mat3x4(v_4, v_6, tint_bitcast_to_f16(mix(v_7.xy, v_7.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
Inner v_8(uint start_byte_offset) {
  return Inner(v_2(start_byte_offset));
}
Inner[4] v_9(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))));
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      a[v_11] = v_8((start_byte_offset + (v_11 * 64u)));
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  return a;
}
Outer v_12(uint start_byte_offset) {
  return Outer(v_9(start_byte_offset));
}
Outer[4] v_13(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))));
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a[v_15] = v_12((start_byte_offset + (v_15 * 256u)));
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_16 = (256u * min(uint(i()), 3u));
  uint v_17 = (64u * min(uint(i()), 3u));
  uint v_18 = (8u * min(uint(i()), 2u));
  Outer l_a[4] = v_13(0u);
  Outer l_a_i = v_12(v_16);
  Inner l_a_i_a[4] = v_9(v_16);
  Inner l_a_i_a_i = v_8((v_16 + v_17));
  f16mat3x4 l_a_i_a_i_m = v_2((v_16 + v_17));
  uvec4 v_19 = v.inner[(((v_16 + v_17) + v_18) / 16u)];
  f16vec4 l_a_i_a_i_m_i = tint_bitcast_to_f16(mix(v_19.xy, v_19.zw, bvec2((((((v_16 + v_17) + v_18) % 16u) / 4u) == 2u))));
  uint v_20 = (((v_16 + v_17) + v_18) + (min(uint(i()), 3u) * 2u));
  uvec4 v_21 = v.inner[(v_20 / 16u)];
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(v_21[((v_20 % 16u) / 4u)])[mix(1u, 0u, ((v_20 % 4u) == 0u))];
}
