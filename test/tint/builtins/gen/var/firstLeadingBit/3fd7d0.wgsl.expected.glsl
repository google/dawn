//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec3 inner;
} v;
uvec3 firstLeadingBit_3fd7d0() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 v_2 = mix(uvec3(16u), uvec3(0u), equal((v_1 & uvec3(4294901760u)), uvec3(0u)));
  uvec3 v_3 = (v_1 >> v_2);
  uvec3 v_4 = mix(uvec3(8u), uvec3(0u), equal((v_3 & uvec3(65280u)), uvec3(0u)));
  uvec3 v_5 = (v_3 >> v_4);
  uvec3 v_6 = mix(uvec3(4u), uvec3(0u), equal((v_5 & uvec3(240u)), uvec3(0u)));
  uvec3 v_7 = (v_5 >> v_6);
  uvec3 v_8 = mix(uvec3(2u), uvec3(0u), equal((v_7 & uvec3(12u)), uvec3(0u)));
  uvec3 v_9 = (v_7 >> v_8);
  uvec3 res = mix((v_2 | (v_4 | (v_6 | (v_8 | mix(uvec3(1u), uvec3(0u), equal((v_9 & uvec3(2u)), uvec3(0u))))))), uvec3(4294967295u), equal(v_9, uvec3(0u)));
  return res;
}
void main() {
  v.inner = firstLeadingBit_3fd7d0();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
uvec3 firstLeadingBit_3fd7d0() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v_1 = arg_0;
  uvec3 v_2 = mix(uvec3(16u), uvec3(0u), equal((v_1 & uvec3(4294901760u)), uvec3(0u)));
  uvec3 v_3 = (v_1 >> v_2);
  uvec3 v_4 = mix(uvec3(8u), uvec3(0u), equal((v_3 & uvec3(65280u)), uvec3(0u)));
  uvec3 v_5 = (v_3 >> v_4);
  uvec3 v_6 = mix(uvec3(4u), uvec3(0u), equal((v_5 & uvec3(240u)), uvec3(0u)));
  uvec3 v_7 = (v_5 >> v_6);
  uvec3 v_8 = mix(uvec3(2u), uvec3(0u), equal((v_7 & uvec3(12u)), uvec3(0u)));
  uvec3 v_9 = (v_7 >> v_8);
  uvec3 res = mix((v_2 | (v_4 | (v_6 | (v_8 | mix(uvec3(1u), uvec3(0u), equal((v_9 & uvec3(2u)), uvec3(0u))))))), uvec3(4294967295u), equal(v_9, uvec3(0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstLeadingBit_3fd7d0();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(location = 0) flat out uvec3 tint_interstage_location0;
uvec3 firstLeadingBit_3fd7d0() {
  uvec3 arg_0 = uvec3(1u);
  uvec3 v = arg_0;
  uvec3 v_1 = mix(uvec3(16u), uvec3(0u), equal((v & uvec3(4294901760u)), uvec3(0u)));
  uvec3 v_2 = (v >> v_1);
  uvec3 v_3 = mix(uvec3(8u), uvec3(0u), equal((v_2 & uvec3(65280u)), uvec3(0u)));
  uvec3 v_4 = (v_2 >> v_3);
  uvec3 v_5 = mix(uvec3(4u), uvec3(0u), equal((v_4 & uvec3(240u)), uvec3(0u)));
  uvec3 v_6 = (v_4 >> v_5);
  uvec3 v_7 = mix(uvec3(2u), uvec3(0u), equal((v_6 & uvec3(12u)), uvec3(0u)));
  uvec3 v_8 = (v_6 >> v_7);
  uvec3 res = mix((v_1 | (v_3 | (v_5 | (v_7 | mix(uvec3(1u), uvec3(0u), equal((v_8 & uvec3(2u)), uvec3(0u))))))), uvec3(4294967295u), equal(v_8, uvec3(0u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_9 = VertexOutput(vec4(0.0f), uvec3(0u));
  v_9.pos = vec4(0.0f);
  v_9.prevent_dce = firstLeadingBit_3fd7d0();
  return v_9;
}
void main() {
  VertexOutput v_10 = vertex_main_inner();
  gl_Position = vec4(v_10.pos.x, -(v_10.pos.y), ((2.0f * v_10.pos.z) - v_10.pos.w), v_10.pos.w);
  tint_interstage_location0 = v_10.prevent_dce;
  gl_PointSize = 1.0f;
}
