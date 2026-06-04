#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  f16mat4 m;
};

struct Outer {
  Inner a[4];
};

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[64];
} v;
f16vec2 tint_bitcast_to_16bit_1(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_3 = tint_bitcast_to_16bit(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_4 = (8u + start_byte_offset);
  uvec4 v_5 = v.inner[(v_4 / 16u)];
  f16vec4 v_6 = tint_bitcast_to_16bit(mix(v_5.xy, v_5.zw, bvec2((((v_4 & 15u) >> 2u) == 2u))));
  uint v_7 = (16u + start_byte_offset);
  uvec4 v_8 = v.inner[(v_7 / 16u)];
  f16vec4 v_9 = tint_bitcast_to_16bit(mix(v_8.xy, v_8.zw, bvec2((((v_7 & 15u) >> 2u) == 2u))));
  uint v_10 = (24u + start_byte_offset);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  return f16mat4(v_3, v_6, v_9, tint_bitcast_to_16bit(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u)))));
}
Inner v_12(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_13(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))));
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a[v_15] = v_12((start_byte_offset + (v_15 * 64u)));
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  return a;
}
Outer v_16(uint start_byte_offset) {
  return Outer(v_13(start_byte_offset));
}
Outer[4] v_17(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))), Outer(Inner[4](Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))), Inner(f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf))))));
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 4u)) {
        break;
      }
      a[v_19] = v_16((start_byte_offset + (v_19 * 256u)));
      {
        v_18 = (v_19 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v_17(0u);
  Outer l_a_3 = v_16(768u);
  Inner l_a_3_a[4] = v_13(768u);
  Inner l_a_3_a_2 = v_12(896u);
  f16mat4 l_a_3_a_2_m = v_1(896u);
  f16vec4 l_a_3_a_2_m_1 = tint_bitcast_to_16bit(v.inner[56u].zw);
  uvec4 v_20 = v.inner[56u];
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_16bit_1(v_20.z).x;
}
