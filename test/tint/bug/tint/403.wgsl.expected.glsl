#version 310 es

layout(binding = 0, std140)
uniform v_x_20_block_ubo {
  uvec4 inner[1];
} v;
layout(binding = 1, std140)
uniform v_x_26_block_ubo {
  uvec4 inner[1];
} v_1;
mat2 v_2(uint start_byte_offset) {
  uvec4 v_3 = v_1.inner[(start_byte_offset / 16u)];
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_5 = (8u + start_byte_offset);
  uvec4 v_6 = v_1.inner[(v_5 / 16u)];
  return mat2(v_4, uintBitsToFloat(mix(v_6.xy, v_6.zw, bvec2((((v_5 & 15u) >> 2u) == 2u)))));
}
mat2 v_7(uint start_byte_offset) {
  uvec4 v_8 = v.inner[(start_byte_offset / 16u)];
  vec2 v_9 = uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2((((start_byte_offset & 15u) >> 2u) == 2u))));
  uint v_10 = (8u + start_byte_offset);
  uvec4 v_11 = v.inner[(v_10 / 16u)];
  return mat2(v_9, uintBitsToFloat(mix(v_11.xy, v_11.zw, bvec2((((v_10 & 15u) >> 2u) == 2u)))));
}
vec4 main_inner(uint v_12) {
  vec2 indexable[3] = vec2[3](vec2(0.0f), vec2(0.0f), vec2(0.0f));
  mat2 x_23 = v_7(0u);
  mat2 x_28 = v_2(0u);
  uint x_46 = v_12;
  indexable = vec2[3](vec2(-1.0f, 1.0f), vec2(1.0f), vec2(-1.0f));
  vec2 x_51 = indexable[min(x_46, 2u)];
  vec2 x_52 = (mat2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])) * x_51);
  return vec4(x_52.x, x_52.y, 0.0f, 1.0f);
}
void main() {
  vec4 v_13 = main_inner(uint(gl_VertexID));
  gl_Position = vec4(v_13.x, -(v_13.y), ((2.0f * v_13.z) - v_13.w), v_13.w);
  gl_PointSize = 1.0f;
}
