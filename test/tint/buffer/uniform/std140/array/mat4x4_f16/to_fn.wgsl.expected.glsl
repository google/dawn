#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[8];
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_2;
float16_t a(f16mat4 a_1[4]) {
  return a_1[0u][0u].x;
}
float16_t b(f16mat4 m) {
  return m[0u].x;
}
float16_t c(f16vec4 v) {
  return v.x;
}
float16_t d(float16_t f_1) {
  return f_1;
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat4 v_3(uint start_byte_offset) {
  uvec4 v_4 = v_1.inner[(start_byte_offset / 16u)];
  f16vec4 v_5 = tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_6 = v_1.inner[((8u + start_byte_offset) / 16u)];
  f16vec4 v_7 = tint_bitcast_to_f16(mix(v_6.xy, v_6.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_8 = v_1.inner[((16u + start_byte_offset) / 16u)];
  f16vec4 v_9 = tint_bitcast_to_f16(mix(v_8.xy, v_8.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_10 = v_1.inner[((24u + start_byte_offset) / 16u)];
  return f16mat4(v_5, v_7, v_9, tint_bitcast_to_f16(mix(v_10.xy, v_10.zw, bvec2(((((24u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
f16mat4[4] v_11(uint start_byte_offset) {
  f16mat4 a_2[4] = f16mat4[4](f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)), f16mat4(f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 4u)) {
        break;
      }
      a_2[v_13] = v_3((start_byte_offset + (v_13 * 32u)));
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float16_t v_14 = a(v_11(0u));
  float16_t v_15 = (v_14 + b(v_3(32u)));
  float16_t v_16 = (v_15 + c(tint_bitcast_to_f16(v_1.inner[2u].xy).ywxz));
  v_2.inner = (v_16 + d(tint_bitcast_to_f16(v_1.inner[2u].xy).ywxz.x));
}
