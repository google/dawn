#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  f16mat4x2 m;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[64];
} v;
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16mat4x2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_16bit(v.inner[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  f16vec2 v_4 = tint_bitcast_to_16bit(v.inner[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  f16vec2 v_6 = tint_bitcast_to_16bit(v.inner[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (12u + start_byte_offset);
  return f16mat4x2(v_2, v_4, v_6, tint_bitcast_to_16bit(v.inner[(v_7 / 16u)][((v_7 & 15u) >> 2u)]));
}
Inner v_8(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_9(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))));
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
    }
  }
  return a;
}
Outer v_12(uint start_byte_offset) {
  return Outer(v_9(start_byte_offset));
}
Outer[4] v_13(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf))))));
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
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v_13(0u);
  Outer l_a_3 = v_12(768u);
  Inner l_a_3_a[4] = v_9(768u);
  Inner l_a_3_a_2 = v_8(896u);
  f16mat4x2 l_a_3_a_2_m = v_1(896u);
  f16vec2 l_a_3_a_2_m_1 = tint_bitcast_to_16bit(v.inner[56u].y);
  uvec4 v_16 = v.inner[56u];
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_16bit(v_16.y).x;
}
