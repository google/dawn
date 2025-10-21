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
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16mat2 v_1(uint start_byte_offset) {
  f16vec2 v_2 = tint_bitcast_to_f16(v.inner[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return f16mat2(v_2, tint_bitcast_to_f16(v.inner[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}
Inner v_3(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_4(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))));
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 4u)) {
        break;
      }
      a[v_6] = v_3((start_byte_offset + (v_6 * 64u)));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  return a;
}
Outer v_7(uint start_byte_offset) {
  return Outer(v_4(start_byte_offset));
}
Outer[4] v_8(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))), Outer(Inner[4](Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))), Inner(f16mat2(f16vec2(0.0hf), f16vec2(0.0hf))))));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a[v_10] = v_7((start_byte_offset + (v_10 * 256u)));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v_8(0u);
  Outer l_a_3 = v_7(768u);
  Inner l_a_3_a[4] = v_4(768u);
  Inner l_a_3_a_2 = v_3(896u);
  f16mat2 l_a_3_a_2_m = v_1(896u);
  f16vec2 l_a_3_a_2_m_1 = tint_bitcast_to_f16(v.inner[56u].y);
  uvec4 v_11 = v.inner[56u];
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_f16(v_11.y).x;
}
