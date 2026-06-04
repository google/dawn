#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[8];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
f16mat4 p[4] = f16mat4[4](f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_16bit_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_16bit_1(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  f16vec4 v_7 = tint_bitcast_to_16bit_1(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u))));
  uint v_8 = (16u + start_byte_offset);
  uvec4 v_9 = v.inner[(v_8 / 16u)];
  f16vec4 v_10 = tint_bitcast_to_16bit_1(mix(v_9.xy, v_9.zw, bvec2((((v_8 & 15u) >> 2u) == 2u))));
  uint v_11 = (24u + start_byte_offset);
  uvec4 v_12 = v.inner[(v_11 / 16u)];
  return f16mat4(v_4, v_7, v_10, tint_bitcast_to_16bit_1(mix(v_12.xy, v_12.zw, bvec2((((v_11 & 15u) >> 2u) == 2u)))));
}
f16mat4[4] v_13(uint start_byte_offset) {
  f16mat4 a[4] = f16mat4[4](f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      a[v_15] = v_2((start_byte_offset + (v_15 * 32u)));
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = v_13(0u);
  p[1u] = v_2(64u);
  p[1u][0u] = tint_bitcast_to_16bit_1(v.inner[0u].zw).ywxz;
  uvec4 v_16 = v.inner[0u];
  p[1u][0u].x = tint_bitcast_to_16bit(v_16.z).x;
  v_1.inner = p[1u][0u].x;
}
