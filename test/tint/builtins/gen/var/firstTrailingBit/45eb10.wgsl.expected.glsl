//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec2 inner;
} v;
uvec2 firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v_1 = arg_0;
  uvec2 res = mix((mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(2u), equal(((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u))) | mix(uvec2(0u), uvec2(1u), equal((((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(2u), equal(((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u)))) & uvec2(1u)), uvec2(0u))))))), uvec2(4294967295u), equal(((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(2u), equal(((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u)))), uvec2(0u)));
  return res;
}
void main() {
  v.inner = firstTrailingBit_45eb10();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec2 inner;
} v;
uvec2 firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v_1 = arg_0;
  uvec2 res = mix((mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(2u), equal(((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u))) | mix(uvec2(0u), uvec2(1u), equal((((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(2u), equal(((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u)))) & uvec2(1u)), uvec2(0u))))))), uvec2(4294967295u), equal(((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(2u), equal(((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v_1 >> mix(uvec2(0u), uvec2(16u), equal((v_1 & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u)))), uvec2(0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstTrailingBit_45eb10();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

layout(location = 0) flat out uvec2 tint_interstage_location0;
uvec2 firstTrailingBit_45eb10() {
  uvec2 arg_0 = uvec2(1u);
  uvec2 v = arg_0;
  uvec2 res = mix((mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(4u), equal((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u))) | (mix(uvec2(0u), uvec2(2u), equal(((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u))) | mix(uvec2(0u), uvec2(1u), equal((((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(2u), equal(((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u)))) & uvec2(1u)), uvec2(0u))))))), uvec2(4294967295u), equal(((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(2u), equal(((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(4u), equal((((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) >> mix(uvec2(0u), uvec2(8u), equal(((v >> mix(uvec2(0u), uvec2(16u), equal((v & uvec2(65535u)), uvec2(0u)))) & uvec2(255u)), uvec2(0u)))) & uvec2(15u)), uvec2(0u)))) & uvec2(3u)), uvec2(0u)))), uvec2(0u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), uvec2(0u));
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = firstTrailingBit_45eb10();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
