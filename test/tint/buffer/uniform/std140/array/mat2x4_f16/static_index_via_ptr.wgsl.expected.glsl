#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[4];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_1;
f16vec2 tint_bitcast_to_f16_1(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_f16(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uvec4 v_5 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x4(v_4, tint_bitcast_to_f16(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))));
}
f16mat2x4[4] v_6(uint start_byte_offset) {
  f16mat2x4 a[4] = f16mat2x4[4](f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      a[v_8] = v_2((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2x4 l_a[4] = v_6(0u);
  f16mat2x4 l_a_i = v_2(32u);
  f16vec4 l_a_i_i = tint_bitcast_to_f16(v.inner[2u].zw);
  uvec4 v_9 = v.inner[2u];
  v_1.inner = (((tint_bitcast_to_f16_1(v_9.z).x + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
