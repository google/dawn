#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec3 inner;
} v;
ivec3 countTrailingZeros_acfacb() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  uvec3 v_2 = mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)));
  uvec3 v_3 = mix(uvec3(0u), uvec3(8u), equal(((v_1 >> v_2) & uvec3(255u)), uvec3(0u)));
  uvec3 v_4 = mix(uvec3(0u), uvec3(4u), equal((((v_1 >> v_2) >> v_3) & uvec3(15u)), uvec3(0u)));
  uvec3 v_5 = mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> v_2) >> v_3) >> v_4) & uvec3(3u)), uvec3(0u)));
  uvec3 v_6 = mix(uvec3(0u), uvec3(1u), equal((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & uvec3(1u)), uvec3(0u)));
  ivec3 res = ivec3(((v_2 | (v_3 | (v_4 | (v_5 | v_6)))) + mix(uvec3(0u), uvec3(1u), equal(((((v_1 >> v_2) >> v_3) >> v_4) >> v_5), uvec3(0u)))));
  return res;
}
void main() {
  v.inner = countTrailingZeros_acfacb();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec3 inner;
} v;
ivec3 countTrailingZeros_acfacb() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v_1 = uvec3(arg_0);
  uvec3 v_2 = mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)));
  uvec3 v_3 = mix(uvec3(0u), uvec3(8u), equal(((v_1 >> v_2) & uvec3(255u)), uvec3(0u)));
  uvec3 v_4 = mix(uvec3(0u), uvec3(4u), equal((((v_1 >> v_2) >> v_3) & uvec3(15u)), uvec3(0u)));
  uvec3 v_5 = mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> v_2) >> v_3) >> v_4) & uvec3(3u)), uvec3(0u)));
  uvec3 v_6 = mix(uvec3(0u), uvec3(1u), equal((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & uvec3(1u)), uvec3(0u)));
  ivec3 res = ivec3(((v_2 | (v_3 | (v_4 | (v_5 | v_6)))) + mix(uvec3(0u), uvec3(1u), equal(((((v_1 >> v_2) >> v_3) >> v_4) >> v_5), uvec3(0u)))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = countTrailingZeros_acfacb();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec3 prevent_dce;
};

layout(location = 0) flat out ivec3 vertex_main_loc0_Output;
ivec3 countTrailingZeros_acfacb() {
  ivec3 arg_0 = ivec3(1);
  uvec3 v = uvec3(arg_0);
  uvec3 v_1 = mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)));
  uvec3 v_2 = mix(uvec3(0u), uvec3(8u), equal(((v >> v_1) & uvec3(255u)), uvec3(0u)));
  uvec3 v_3 = mix(uvec3(0u), uvec3(4u), equal((((v >> v_1) >> v_2) & uvec3(15u)), uvec3(0u)));
  uvec3 v_4 = mix(uvec3(0u), uvec3(2u), equal(((((v >> v_1) >> v_2) >> v_3) & uvec3(3u)), uvec3(0u)));
  uvec3 v_5 = mix(uvec3(0u), uvec3(1u), equal((((((v >> v_1) >> v_2) >> v_3) >> v_4) & uvec3(1u)), uvec3(0u)));
  ivec3 res = ivec3(((v_1 | (v_2 | (v_3 | (v_4 | v_5)))) + mix(uvec3(0u), uvec3(1u), equal(((((v >> v_1) >> v_2) >> v_3) >> v_4), uvec3(0u)))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec3(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = countTrailingZeros_acfacb();
  return tint_symbol;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = v_6.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
