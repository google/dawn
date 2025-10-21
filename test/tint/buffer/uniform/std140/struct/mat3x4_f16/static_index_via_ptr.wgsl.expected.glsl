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
f16vec2 tint_bitcast_to_f16_1(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat3x4 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_3 = tint_bitcast_to_f16(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_5 = tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_6 = v.inner[((16u + start_byte_offset) / 16u)];
  return f16mat3x4(v_3, v_5, tint_bitcast_to_f16(mix(v_6.xy, v_6.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
Inner v_7(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_8(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))));
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
      continue;
    }
  }
  return a;
}
Outer v_11(uint start_byte_offset) {
  return Outer(v_8(start_byte_offset));
}
Outer[4] v_12(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat3x4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))));
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
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v_12(0u);
  Outer l_a_3 = v_11(768u);
  Inner l_a_3_a[4] = v_8(768u);
  Inner l_a_3_a_2 = v_7(896u);
  f16mat3x4 l_a_3_a_2_m = v_1(896u);
  f16vec4 l_a_3_a_2_m_1 = tint_bitcast_to_f16(v.inner[56u].zw);
  uvec4 v_15 = v.inner[56u];
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_f16_1(v_15.z).x;
}
