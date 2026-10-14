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
  Outer l_a[4] = v_8(0u);
  Outer l_a_3 = v_7(768u);
  Inner l_a_3_a[4] = v_5(768u);
  Inner l_a_3_a_2 = v_4(896u);
  f16mat2 l_a_3_a_2_m = v_1(896u);
  f16vec2 l_a_3_a_2_m_1 = tint_bitcast_to_16bit(v.inner[56u].y);
  uvec4 v_10 = v.inner[56u];
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_16bit(v_10.y).x;
}
