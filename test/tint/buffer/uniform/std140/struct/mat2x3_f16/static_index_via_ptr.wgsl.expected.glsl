#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  f16mat2x3 m;
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
f16mat2x3 v_1(uint start_byte_offset) {
  uvec4 v_2 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_3 = tint_bitcast_to_f16(mix(v_2.xy, v_2.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u)))).xyz;
  uvec4 v_4 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x3(v_3, tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))).xyz);
}
Inner v_5(uint start_byte_offset) {
  return Inner(v_1(start_byte_offset));
}
Inner[4] v_6(uint start_byte_offset) {
  Inner a[4] = Inner[4](Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_5((start_byte_offset + (v_8 * 64u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  return a;
}
Outer v_9(uint start_byte_offset) {
  return Outer(v_6(start_byte_offset));
}
Outer[4] v_10(uint start_byte_offset) {
  Outer a[4] = Outer[4](Outer(Inner[4](Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))))), Outer(Inner[4](Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))))), Outer(Inner[4](Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))))), Outer(Inner[4](Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))), Inner(f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf))))));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_9((start_byte_offset + (v_12 * 256u)));
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  Outer l_a[4] = v_10(0u);
  Outer l_a_3 = v_9(768u);
  Inner l_a_3_a[4] = v_6(768u);
  Inner l_a_3_a_2 = v_5(896u);
  f16mat2x3 l_a_3_a_2_m = v_1(896u);
  f16vec3 l_a_3_a_2_m_1 = tint_bitcast_to_f16(v.inner[56u].zw).xyz;
  uvec4 v_13 = v.inner[56u];
  float16_t l_a_3_a_2_m_1_0 = tint_bitcast_to_f16_1(v_13.z).x;
}
