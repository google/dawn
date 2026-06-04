#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  f16mat4x3 m;
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
f16mat4x3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_16bit(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  f16vec3 v_7 = tint_bitcast_to_16bit(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))).xyz;
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v.inner[(v_8 / 16u)];
  f16vec3 v_10 = tint_bitcast_to_16bit(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u)))).xyz;
  uint v_11 = (24u + start_byte_offset);
  uvec4 v_12 = v.inner[(v_11 / 16u)];
  return f16mat4x3(v_4, v_7, v_10, tint_bitcast_to_16bit(mix(v_12.xy, v_12.zw, bvec2((((v_11 & 15u) >> 2u) == 2u)))).xyz);
}
Inner v_13(uint start_byte_offset) {
  return Inner(v_2(start_byte_offset));
}
Inner[4] v_14(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))));
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      a[v_16] = v_13((start_byte_offset + (v_16 * 64u)));
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
  return a;
}
Outer v_17(uint start_byte_offset) {
  return Outer(v_14(start_byte_offset));
}
Outer[4] v_18(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))))), Outer(Inner[4](Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))))), Outer(Inner[4](Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))))), Outer(Inner[4](Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf))))));
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 4u)) {
        break;
      }
      a[v_20] = v_17((start_byte_offset + (v_20 * 256u)));
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_21 = (min(uint(i()), 3u) * 256u);
  uint v_22 = (min(uint(i()), 3u) * 64u);
  uint v_23 = (min(uint(i()), 3u) * 8u);
  Outer l_a[4] = v_18(0u);
  Outer l_a_i = v_17(v_21);
  Inner l_a_i_a[4] = v_14(v_21);
  Inner l_a_i_a_i = v_13((v_21 + v_22));
  f16mat4x3 l_a_i_a_i_m = v_2((v_21 + v_22));
  uint v_24 = ((v_21 + v_22) + v_23);
  uvec4 v_25 = v.inner[(v_24 / 16u)];
  f16vec3 l_a_i_a_i_m_i = tint_bitcast_to_16bit(mix(v_25.xy, v_25.zw, bvec2((((v_24 & 15u) >> 2u) == 2u)))).xyz;
  uint v_26 = (((v_21 + v_22) + v_23) + (min(uint(i()), 2u) * 2u));
  uvec4 v_27 = v.inner[(v_26 / 16u)];
  float16_t l_a_i_a_i_m_i_i = tint_bitcast_to_16bit_1(v_27[((v_26 & 15u) >> 2u)])[mix(1u, 0u, ((v_26 % 4u) == 0u))];
}
