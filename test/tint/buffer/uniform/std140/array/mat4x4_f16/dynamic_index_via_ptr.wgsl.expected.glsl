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
f16vec2 tint_bitcast_to_16bit_1(uint src) {
  return unpackFloat2x16(src);
}
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_3(uint start_byte_offset) {
  uvec4 v_4 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_5 = tint_bitcast_to_16bit(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_6 = (8u + start_byte_offset);
  uvec4 v_7 = v.inner[(v_6 / 16u)];
  f16vec4 v_8 = tint_bitcast_to_16bit(mix(v_7.xy, v_7.zw, bvec2((((v_6 & 15u) >> 2u) == 2u))));
  uint v_9 = (16u + start_byte_offset);
  uvec4 v_10 = v.inner[(v_9 / 16u)];
  f16vec4 v_11 = tint_bitcast_to_16bit(mix(v_10.xy, v_10.zw, bvec2((((v_9 & 15u) >> 2u) == 2u))));
  uint v_12 = (24u + start_byte_offset);
  uvec4 v_13 = v.inner[(v_12 / 16u)];
  return f16mat4(v_5, v_8, v_11, tint_bitcast_to_16bit(mix(v_13.xy, v_13.zw, bvec2((((v_12 & 15u) >> 2u) == 2u)))));
}
f16mat4[4] v_14(uint start_byte_offset) {
  f16mat4 a[4] = f16mat4[4](f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 4u)) {
        break;
      }
      a[v_16] = v_3((start_byte_offset + (v_16 * 32u)));
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
  return a;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  uint v_17 = (min(uint(i()), 3u) * 32u);
  uint v_18 = (min(uint(i()), 3u) * 8u);
  f16mat4 l_a[4] = v_14(0u);
  f16mat4 l_a_i = v_3(v_17);
  uint v_19 = (v_17 + v_18);
  uvec4 v_20 = v.inner[(v_19 / 16u)];
  f16vec4 l_a_i_i = tint_bitcast_to_16bit(mix(v_20.xy, v_20.zw, bvec2((((v_19 & 15u) >> 2u) == 2u))));
  uint v_21 = (v_17 + v_18);
  uvec4 v_22 = v.inner[(v_21 / 16u)];
  v_1.inner = (((tint_bitcast_to_16bit_1(v_22[((v_21 & 15u) >> 2u)])[mix(1u, 0u, ((v_21 % 4u) == 0u))] + l_a[0u][0u].x) + l_a_i[0u].x) + l_a_i_i.x);
}
