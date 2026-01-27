#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  f16mat2x4 m;
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
f16mat2x4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_f16(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x4(v_4, tint_bitcast_to_f16(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
Inner v_6(uint start_byte_offset) {
  return Inner(v_2(start_byte_offset));
}
Inner[4] v_7(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_6((start_byte_offset + (v_9 * 64u)));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  return a;
}
Outer v_10(uint start_byte_offset) {
  return Outer(v_7(start_byte_offset));
}
Outer[4] v_11(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf))))));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = v_10((start_byte_offset + (v_13 * 256u)));
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_14 = (min(uint(i()), 3u) * 256u);
  uint v_15 = (min(uint(i()), 3u) * 64u);
  uint v_16 = (min(uint(i()), 1u) * 8u);
  Outer l_a[4] = v_11(0u);
  Outer l_a_i = v_10(v_14);
  Inner l_a_i_a[4] = v_7(v_14);
  Inner l_a_i_a_i = v_6((v_14 + v_15));
  f16mat2x4 l_a_i_a_i_m = v_2((v_14 + v_15));
  uvec4 v_17 = v.inner[(((v_14 + v_15) + v_16) / 16u)];
  f16vec4 l_a_i_a_i_m_i = tint_bitcast_to_f16(mix(v_17.xy, v_17.zw, bvec2((((((v_14 + v_15) + v_16) & 15u) >> 2u) == 2u))));
  uint v_18 = (((v_14 + v_15) + v_16) + (min(uint(i()), 3u) * 2u));
  uvec4 v_19 = v.inner[(v_18 / 16u)];
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16_1(v_19[((v_18 & 15u) >> 2u)])[mix(1u, 0u, ((v_18 % 4u) == 0u))];
}
