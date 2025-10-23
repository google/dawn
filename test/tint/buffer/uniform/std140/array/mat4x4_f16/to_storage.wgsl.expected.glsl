#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[8];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  f16mat4 inner[4];
} v_1;
f16vec2 tint_bitcast_to_f16(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_f16_1(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_6 = tint_bitcast_to_f16_1(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_7 = v.inner[((16u + start_byte_offset) / 16u)];
  f16vec4 v_8 = tint_bitcast_to_f16_1(mix(v_7.xy, v_7.zw, bvec2(((((16u + start_byte_offset) & 15u) >> 2u) == 2u))));
  uvec4 v_9 = v.inner[((24u + start_byte_offset) / 16u)];
  return f16mat4(v_4, v_6, v_8, tint_bitcast_to_f16_1(mix(v_9.xy, v_9.zw, bvec2(((((24u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
f16mat4[4] v_10(uint start_byte_offset) {
  f16mat4 a[4] = f16mat4[4](f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      a[v_12] = v_2((start_byte_offset + (v_12 * 32u)));
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
  v_1.inner = v_10(0u);
  v_1.inner[1u] = v_2(64u);
  v_1.inner[1u][0u] = tint_bitcast_to_f16_1(v.inner[0u].zw).ywxz;
  uvec4 v_13 = v.inner[0u];
  v_1.inner[1u][0u].x = tint_bitcast_to_f16(v_13.z).x;
}
