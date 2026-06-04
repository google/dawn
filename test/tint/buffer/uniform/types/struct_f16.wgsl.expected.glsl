#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct Inner {
  float16_t scalar_f16;
  uint tint_pad_0;
  f16vec3 vec3_f16;
  f16mat2x4 mat2x4_f16;
};

struct S {
  Inner inner;
};

layout(binding = 0, std140)
uniform u_block_1_ubo {
  uvec4 inner[2];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  S inner;
} v_1;
void tint_store_and_preserve_padding_1(Inner value_param) {
  v_1.inner.inner.scalar_f16 = value_param.scalar_f16;
  v_1.inner.inner.vec3_f16 = value_param.vec3_f16;
  v_1.inner.inner.mat2x4_f16 = value_param.mat2x4_f16;
}
void tint_store_and_preserve_padding(S value_param) {
  tint_store_and_preserve_padding_1(value_param.inner);
}
f16vec4 tint_bitcast_to_16bit_1(uvec2 src) {
  return f16vec4(unpackFloat2x16(src.x), unpackFloat2x16(src.y));
}
f16mat2x4 v_2(uint start_byte_offset) {
  uvec4 v_3 = v.inner[(start_byte_offset / 16u)];
  f16vec4 v_4 = tint_bitcast_to_16bit_1(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v.inner[(v_5 / 16u)];
  return f16mat2x4(v_4, tint_bitcast_to_16bit_1(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))));
}
f16vec2 tint_bitcast_to_16bit(uint src) {
  return unpackFloat2x16(src);
}
Inner v_7(uint start_byte_offset) {
  uvec4 v_8 = v.inner[(start_byte_offset / 16u)];
  float16_t v_9 = tint_bitcast_to_16bit(v_8[((start_byte_offset & 15u) >> 2u)])[mix(1u, 0u, ((start_byte_offset % 4u) == 0u))];
  uint v_10 = (8u + start_byte_offset);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  f16vec3 v_12 = tint_bitcast_to_16bit_1(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u)))).xyz;
  return Inner(v_9, 0u, v_12, v_2((16u + start_byte_offset)));
}
S v_13(uint start_byte_offset) {
  return S(v_7(start_byte_offset));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  S x = v_13(0u);
  tint_store_and_preserve_padding(x);
}
