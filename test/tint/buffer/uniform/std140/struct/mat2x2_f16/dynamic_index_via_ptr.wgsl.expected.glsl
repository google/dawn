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
  uint v_1 = uint(counter);
  counter = int((v_1 + uint(1)));
  return counter;
}
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat2 v_2(uint start_byte_offset) {
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return f16mat2(v_3, tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}
Inner v_4(uint start_byte_offset) {
  return Inner(v_2(start_byte_offset));
}
Inner[4] v_5(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))));
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 4u)) {
        break;
      }
      a[v_7] = v_4((start_byte_offset + (v_7 * 64u)));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  return a;
}
Outer v_8(uint start_byte_offset) {
  return Outer(v_5(start_byte_offset));
}
Outer[4] v_9(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))));
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      a[v_11] = v_8((start_byte_offset + (v_11 * 256u)));
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_12 = (256u * min(uint(i()), 3u));
  uint v_13 = (64u * min(uint(i()), 3u));
  uint v_14 = (4u * min(uint(i()), 1u));
  Outer l_a[4] = v_9(0u);
  Outer l_a_i = v_8(v_12);
  Inner l_a_i_a[4] = v_5(v_12);
  Inner l_a_i_a_i = v_4((v_12 + v_13));
  f16mat2 l_a_i_a_i_m = v_2((v_12 + v_13));
  f16vec2 l_a_i_a_i_m_i = tint_bitcast_to_f16(v.inner[(((v_12 + v_13) + v_14) / 16u)][((((v_12 + v_13) + v_14) % 16u) / 4u)]);
  uint v_15 = (((v_12 + v_13) + v_14) + (min(uint(i()), 1u) * 2u));
  uvec4 v_16 = v.inner[(v_15 / 16u)];
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_f16(v_16[((v_15 % 16u) / 4u)])[mix(1u, 0u, ((v_15 % 4u) == 0u))];
}
