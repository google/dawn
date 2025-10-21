#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform a_block_1_ubo {
  uvec4 inner[8];
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
f16mat4 v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_5 = tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_6 = v.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_7 = tint_bitcast_to_f16(mix(v_6.xy, v_6.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_8 = v.inner[((16u + start_byte_offset) / 16u)];
  f16vec4 v_9 = tint_bitcast_to_f16(mix(v_8.xy, v_8.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_10 = v.inner[((24u + start_byte_offset) / 16u)];
  return f16mat4(v_5, v_7, v_9, tint_bitcast_to_f16(mix(v_10.xy, v_10.zw, bvec2(((((24u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
f16mat4[4] v_11(uint start_byte_offset) {
  f16mat4 a[4] = f16mat4[4](f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = v_3((start_byte_offset + (v_13 * 32u)));
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_14 = (32u * min(uint(i()), 3u));
  uint v_15 = (8u * min(uint(i()), 3u));
  f16mat4 l_a[4] = v_11(0u);
  f16mat4 l_a_i = v_3(v_14);
  uvec4 v_16 = v.inner[((v_14 + v_15) / 16u)];
  f16vec4 l_a_i_i = tint_bitcast_to_f16(mix(v_16.xy, v_16.zw, bvec2(((((v_14 + v_15) % 16u) / 4u) == 2u))));
  uvec4 v_17 = v.inner[((v_14 + v_15) / 16u)];
  v_1.inner = (((tint_bitcast_to_f16_1(v_17[(((v_14 + v_15) % 16u) / 4u)])[mix(1u, 0u, (((v_14 + v_15) % 4u) == 0u))] + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
