#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[8];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16mat4x3 inner[4];
} v_1;
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_16bit_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
void tint_store_and_preserve_padding_1(uint target_indices[1], f16mat4x3 value_param) {
  v_1.inner[target_indices[0u]][0u] = value_param[0u];
  v_1.inner[target_indices[0u]][1u] = value_param[1u];
  v_1.inner[target_indices[0u]][2u] = value_param[2u];
  v_1.inner[target_indices[0u]][3u] = value_param[3u];
}
f16mat4x3 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_4 = tint_bitcast_to_16bit_1(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  f16vec3 v_7 = tint_bitcast_to_16bit_1(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))).xyz;
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v.inner[(v_8 / 16u)];
  f16vec3 v_10 = tint_bitcast_to_16bit_1(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u)))).xyz;
  uint v_11 = (24u + start_byte_offset);
  uvec4 v_12 = v.inner[(v_11 / 16u)];
  return f16mat4x3(v_4, v_7, v_10, tint_bitcast_to_16bit_1(mix(v_12.xy, v_12.zw, bvec2((((v_11 & 15u) >> 2u) == 2u)))).xyz);
}
void tint_store_and_preserve_padding(f16mat4x3 value_param[4]) {
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      tint_store_and_preserve_padding_1(uint[1](v_14), value_param[v_14]);
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
}
f16mat4x3[4] v_15(uint start_byte_offset) {
  f16mat4x3 a[4] = f16mat4x3[4](f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)), f16mat4x3(f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf), f16vec3(0.0hf)));
  {
    uint v_16 = 0u;
    v_16 = 0u;
    while(true) {
      uint v_17 = v_16;
      if ((v_17 >= 4u)) {
        break;
      }
      a[v_17] = v_2((start_byte_offset + (v_17 * 32u)));
      {
        v_16 = (v_17 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_store_and_preserve_padding(v_15(0u));
  f16mat4x3 v_18 = v_2(64u);
  tint_store_and_preserve_padding_1(uint[1](1u), v_18);
  v_1.inner[1u][0u] = tint_bitcast_to_16bit_1(v.inner[0u].zw).xyz.zxy;
  uvec4 v_19 = v.inner[0u];
  v_1.inner[1u][0u].x = tint_bitcast_to_16bit(v_19.z).x;
}
