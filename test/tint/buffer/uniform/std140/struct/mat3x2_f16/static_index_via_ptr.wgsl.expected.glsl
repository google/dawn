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
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat3x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  f16vec2 v_3 = tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  return f16mat3x2(v_2, v_3, tint_bitcast_to_f16(v.inner[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]));
}
Inner v_4(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_5(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))));
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
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))));
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
  Outer l_a[4] = v_9(0u);
  Outer l_a_3 = v_8(768u);
  Inner l_a_3_a[4] = v_5(768u);
  Inner l_a_3_a_2 = v_4(896u);
  f16mat3x2 l_a_3_a_2_m = v_1(896u);
  f16vec2 l_a_3_a_2_m_1 = tint_bitcast_to_f16(v.inner[56u].y);
  uvec4 v_12 = v.inner[56u];
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_f16(v_12.y).x;
}
