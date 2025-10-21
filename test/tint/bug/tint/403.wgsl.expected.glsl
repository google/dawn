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
  vec2 v_4 = uintBitsToFloat(mix(v_3.xy, v_3.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_5 = v_1.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_4, uintBitsToFloat(mix(v_5.xy, v_5.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat2 v_6(uint start_byte_offset) {
  uvec4 v_7 = v.inner[(start_byte_offset / 16u)];
  vec2 v_8 = uintBitsToFloat(mix(v_7.xy, v_7.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_9 = v.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_8, uintBitsToFloat(mix(v_9.xy, v_9.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
vec4 main_inner(uint v_10) {
  vec2 indexable[3] = vec2[3](vec2(0.0f), vec2(0.0f), vec2(0.0f));
  mat2 x_23 = v_6(0u);
  mat2 x_28 = v_2(0u);
  uint x_46 = v_10;
  indexable = vec2[3](vec2(-1.0f, 1.0f), vec2(1.0f), vec2(-1.0f));
  vec2 x_51 = indexable[min(x_46, 2u)];
  vec2 x_52 = (mat2((x_23[0u] + x_28[0u]), (x_23[1u] + x_28[1u])) * x_51);
  return vec4(x_52.x, x_52.y, 0.0f, 1.0f);
}
void main() {
  vec4 v_11 = main_inner(uint(gl_VertexID));
  gl_Position = vec4(v_11.x, -(v_11.y), ((2.0f * v_11.z) - v_11.w), v_11.w);
  gl_PointSize = 1.0f;
}
