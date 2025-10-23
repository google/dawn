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
int counter = 0;
int i() {
  uint v_2 = uint(counter);
  counter = int((v_2 + uint(1)));
  return counter;
}
f16vec2 tint_bitcast_to_f16_1(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x3 v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  f16vec3 v_5 = tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_6 = v.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x3(v_5, tint_bitcast_to_f16(mix(v_6.xy, v_6.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz);
}
f16mat2x3[4] v_7(uint start_byte_offset) {
  f16mat2x3 a[4] = f16mat2x3[4](f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a[v_9] = v_3((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_10 = (16u * min(uint(i()), 3u));
  uint v_11 = (8u * min(uint(i()), 1u));
  f16mat2x3 l_a[4] = v_7(0u);
  f16mat2x3 l_a_i = v_3(v_10);
  uvec4 v_12 = v.inner[((v_10 + v_11) / 16u)];
  f16vec3 l_a_i_i = tint_bitcast_to_f16(mix(v_12.xy, v_12.zw, bvec2(((((v_10 + v_11) & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_13 = v.inner[((v_10 + v_11) / 16u)];
  v_1.inner = (((tint_bitcast_to_f16_1(v_13[(((v_10 + v_11) & 15u) >> 2u)])[mix(1u, 0u, (((v_10 + v_11) % 4u) == 0u))] + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
