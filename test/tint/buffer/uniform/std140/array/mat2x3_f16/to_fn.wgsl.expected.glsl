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
float16_t a(f16mat2x3 a_1[4]) {
  return a_1[0u][0u].x;
}
float16_t b(f16mat2x3 m) {
  return m[0u].x;
}
float16_t c(f16vec3 v) {
  return v.x;
}
float16_t d(float16_t f_1) {
  return f_1;
}
f16vec4 tint_bitcast_to_f16(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x3 v_3(uint start_byte_offset) {
  uvec4 v_4 = v_1.inner[(start_byte_offset / 16u)];
  f16vec3 v_5 = tint_bitcast_to_f16(mix(v_4.xy, v_4.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u)))).xyz;
  uvec4 v_6 = v_1.inner[((8u + start_byte_offset) / 16u)];
  return f16mat2x3(v_5, tint_bitcast_to_f16(mix(v_6.xy, v_6.zw, bvec2(((((8u + start_byte_offset) & 15u) >> 2u) == 2u)))).xyz);
}
f16mat2x3[4] v_7(uint start_byte_offset) {
  f16mat2x3 a_2[4] = f16mat2x3[4](f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
  {
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      a_2[v_9] = v_3((start_byte_offset + (v_9 * 16u)));
      {
        v_8 = (v_9 + 1u);
      }
      continue;
    }
  }
  return a_2;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float16_t v_10 = a(v_7(0u));
  float16_t v_11 = (v_10 + b(v_3(16u)));
  float16_t v_12 = (v_11 + c(tint_bitcast_to_f16(v_1.inner[1u].xy).xyz.zxy));
  v_2.inner = (v_12 + d(tint_bitcast_to_f16(v_1.inner[1u].xy).xyz.zxy.x));
}
