//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  int inner;
} v;
int firstLeadingBit_57a1a3() {
  int arg_0 = 1;
  uint v_1 = uint(arg_0);
  uint v_2 = mix(~(v_1), v_1, (v_1 < 2147483648u));
  uint v_3 = mix(16u, 0u, ((v_2 & 4294901760u) == 0u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = mix(8u, 0u, ((v_4 & 65280u) == 0u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = mix(4u, 0u, ((v_6 & 240u) == 0u));
  uint v_8 = (v_6 >> v_7);
  uint v_9 = mix(2u, 0u, ((v_8 & 12u) == 0u));
  uint v_10 = (v_8 >> v_9);
  int res = int(mix((v_3 | (v_5 | (v_7 | (v_9 | mix(1u, 0u, ((v_10 & 2u) == 0u)))))), 4294967295u, (v_10 == 0u)));
  return res;
}
void main() {
  v.inner = firstLeadingBit_57a1a3();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  int inner;
} v;
int firstLeadingBit_57a1a3() {
  int arg_0 = 1;
  uint v_1 = uint(arg_0);
  uint v_2 = mix(~(v_1), v_1, (v_1 < 2147483648u));
  uint v_3 = mix(16u, 0u, ((v_2 & 4294901760u) == 0u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = mix(8u, 0u, ((v_4 & 65280u) == 0u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = mix(4u, 0u, ((v_6 & 240u) == 0u));
  uint v_8 = (v_6 >> v_7);
  uint v_9 = mix(2u, 0u, ((v_8 & 12u) == 0u));
  uint v_10 = (v_8 >> v_9);
  int res = int(mix((v_3 | (v_5 | (v_7 | (v_9 | mix(1u, 0u, ((v_10 & 2u) == 0u)))))), 4294967295u, (v_10 == 0u)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = firstLeadingBit_57a1a3();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int tint_interstage_location0;
int firstLeadingBit_57a1a3() {
  int arg_0 = 1;
  uint v = uint(arg_0);
  uint v_1 = mix(~(v), v, (v < 2147483648u));
  uint v_2 = mix(16u, 0u, ((v_1 & 4294901760u) == 0u));
  uint v_3 = (v_1 >> v_2);
  uint v_4 = mix(8u, 0u, ((v_3 & 65280u) == 0u));
  uint v_5 = (v_3 >> v_4);
  uint v_6 = mix(4u, 0u, ((v_5 & 240u) == 0u));
  uint v_7 = (v_5 >> v_6);
  uint v_8 = mix(2u, 0u, ((v_7 & 12u) == 0u));
  uint v_9 = (v_7 >> v_8);
  int res = int(mix((v_2 | (v_4 | (v_6 | (v_8 | mix(1u, 0u, ((v_9 & 2u) == 0u)))))), 4294967295u, (v_9 == 0u)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_10 = VertexOutput(vec4(0.0f), 0);
  v_10.pos = vec4(0.0f);
  v_10.prevent_dce = firstLeadingBit_57a1a3();
  return v_10;
}
void main() {
  VertexOutput v_11 = vertex_main_inner();
  gl_Position = vec4(v_11.pos.x, -(v_11.pos.y), ((2.0f * v_11.pos.z) - v_11.pos.w), v_11.pos.w);
  tint_interstage_location0 = v_11.prevent_dce;
  gl_PointSize = 1.0f;
}
