#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[4];
} v_1;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  float16_t inner;
} v_2;
float16_t a(f16mat2x4 a_1[4]) {
  return a_1[0u][0u].x;
}
float16_t b(f16mat2x4 m) {
  return m[0u].x;
}
float16_t c(f16vec4 v) {
  return v.x;
}
float16_t d(float16_t f_1) {
  return f_1;
}
f16vec4 tint_bitcast_to_16bit(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x4 v_3(uint start_byte_offset) {
  uvec4 v_4 = v_1.inner[(start_byte_offset / 16u)];
  f16vec4 v_5 = tint_bitcast_to_16bit(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_6 = (8u + start_byte_offset);
  uvec4 v_7 = v_1.inner[(v_6 / 16u)];
  return f16mat2x4(v_5, tint_bitcast_to_16bit(mix(v_7.xy, v_7.zw, bvec2((((v_6 & 15u) >> 2u) == 2u)))));
}
f16mat2x4[4] v_8(uint start_byte_offset) {
  f16mat2x4 a_2[4] = f16mat2x4[4](f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)), f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf)));
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      a_2[v_10] = v_3((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float16_t v_11 = a(v_8(0u));
  float16_t v_12 = (v_11 + b(v_3(16u)));
  float16_t v_13 = (v_12 + c(tint_bitcast_to_16bit(v_1.inner[1u].xy).ywxz));
  v_2.inner = (v_13 + d(tint_bitcast_to_16bit(v_1.inner[1u].xy).ywxz.x));
}
