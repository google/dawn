//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 res = mix((mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u))) | mix(uvec3(0u), uvec3(1u), equal((((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))) & uvec3(1u)), uvec3(0u))))))), uvec3(4294967295u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))), uvec3(0u)));
  return res;
}
void main() {
  v.inner = firstTrailingBit_cb51ce();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 res = mix((mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u))) | mix(uvec3(0u), uvec3(1u), equal((((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))) & uvec3(1u)), uvec3(0u))))))), uvec3(4294967295u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v_1 >> mix(uvec3(0u), uvec3(16u), equal((v_1 & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))), uvec3(0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstTrailingBit_cb51ce();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 vertex_main_loc0_Output;
uvec3 firstTrailingBit_cb51ce() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v = arg_0;
  uvec3 res = mix((mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u))) | (mix(uvec3(0u), uvec3(2u), equal(((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u))) | mix(uvec3(0u), uvec3(1u), equal((((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))) & uvec3(1u)), uvec3(0u))))))), uvec3(4294967295u), equal(((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(2u), equal(((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(4u), equal((((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) >> mix(uvec3(0u), uvec3(8u), equal(((v >> mix(uvec3(0u), uvec3(16u), equal((v & uvec3(65535u)), uvec3(0u)))) & uvec3(255u)), uvec3(0u)))) & uvec3(15u)), uvec3(0u)))) & uvec3(3u)), uvec3(0u)))), uvec3(0u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = firstTrailingBit_cb51ce();
  return tint_symbol;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
