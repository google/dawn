//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uint inner;
} v;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = mix(16u, 0u, ((v_1 & 4294901760u) == 0u));
  uint v_3 = (v_1 >> v_2);
  uint v_4 = mix(8u, 0u, ((v_3 & 65280u) == 0u));
  uint v_5 = (v_3 >> v_4);
  uint v_6 = mix(4u, 0u, ((v_5 & 240u) == 0u));
  uint v_7 = (v_5 >> v_6);
  uint v_8 = mix(2u, 0u, ((v_7 & 12u) == 0u));
  uint v_9 = (v_7 >> v_8);
  uint res = mix((v_2 | (v_4 | (v_6 | (v_8 | mix(1u, 0u, ((v_9 & 2u) == 0u)))))), 4294967295u, (v_9 == 0u));
  return res;
}
void main() {
  v.inner = firstLeadingBit_f0779d();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = mix(16u, 0u, ((v_1 & 4294901760u) == 0u));
  uint v_3 = (v_1 >> v_2);
  uint v_4 = mix(8u, 0u, ((v_3 & 65280u) == 0u));
  uint v_5 = (v_3 >> v_4);
  uint v_6 = mix(4u, 0u, ((v_5 & 240u) == 0u));
  uint v_7 = (v_5 >> v_6);
  uint v_8 = mix(2u, 0u, ((v_7 & 12u) == 0u));
  uint v_9 = (v_7 >> v_8);
  uint res = mix((v_2 | (v_4 | (v_6 | (v_8 | mix(1u, 0u, ((v_9 & 2u) == 0u)))))), 4294967295u, (v_9 == 0u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstLeadingBit_f0779d();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

layout(location = 0) flat out uint tint_interstage_location0;
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = mix(16u, 0u, ((v & 4294901760u) == 0u));
  uint v_2 = (v >> v_1);
  uint v_3 = mix(8u, 0u, ((v_2 & 65280u) == 0u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = mix(4u, 0u, ((v_4 & 240u) == 0u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = mix(2u, 0u, ((v_6 & 12u) == 0u));
  uint v_8 = (v_6 >> v_7);
  uint res = mix((v_1 | (v_3 | (v_5 | (v_7 | mix(1u, 0u, ((v_8 & 2u) == 0u)))))), 4294967295u, (v_8 == 0u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_9 = VertexOutput(vec4(0.0f), 0u);
  v_9.pos = vec4(0.0f);
  v_9.prevent_dce = firstLeadingBit_f0779d();
  return v_9;
}
void main() {
  VertexOutput v_10 = vertex_main_inner();
  gl_Position = vec4(v_10.pos.x, -(v_10.pos.y), ((2.0f * v_10.pos.z) - v_10.pos.w), v_10.pos.w);
  tint_interstage_location0 = v_10.prevent_dce;
  gl_PointSize = 1.0f;
}
