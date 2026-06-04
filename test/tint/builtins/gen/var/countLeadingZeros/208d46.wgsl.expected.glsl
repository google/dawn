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
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = mix(0u, 16u, (v_1 <= 65535u));
  uint v_3 = (v_1 << v_2);
  uint v_4 = mix(0u, 8u, (v_3 <= 16777215u));
  uint v_5 = (v_3 << v_4);
  uint v_6 = mix(0u, 4u, (v_5 <= 268435455u));
  uint v_7 = (v_5 << v_6);
  uint v_8 = mix(0u, 2u, (v_7 <= 1073741823u));
  uint v_9 = (v_7 << v_8);
  uint v_10 = mix(0u, 1u, (v_9 == 0u));
  uint res = ((v_2 | (v_4 | (v_6 | (v_8 | (mix(0u, 1u, (v_9 <= 2147483647u)) | v_10))))) + v_10);
  return res;
}
void main() {
  v.inner = countLeadingZeros_208d46();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v_1 = arg_0;
  uint v_2 = mix(0u, 16u, (v_1 <= 65535u));
  uint v_3 = (v_1 << v_2);
  uint v_4 = mix(0u, 8u, (v_3 <= 16777215u));
  uint v_5 = (v_3 << v_4);
  uint v_6 = mix(0u, 4u, (v_5 <= 268435455u));
  uint v_7 = (v_5 << v_6);
  uint v_8 = mix(0u, 2u, (v_7 <= 1073741823u));
  uint v_9 = (v_7 << v_8);
  uint v_10 = mix(0u, 1u, (v_9 == 0u));
  uint res = ((v_2 | (v_4 | (v_6 | (v_8 | (mix(0u, 1u, (v_9 <= 2147483647u)) | v_10))))) + v_10);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = countLeadingZeros_208d46();
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
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = mix(0u, 16u, (v <= 65535u));
  uint v_2 = (v << v_1);
  uint v_3 = mix(0u, 8u, (v_2 <= 16777215u));
  uint v_4 = (v_2 << v_3);
  uint v_5 = mix(0u, 4u, (v_4 <= 268435455u));
  uint v_6 = (v_4 << v_5);
  uint v_7 = mix(0u, 2u, (v_6 <= 1073741823u));
  uint v_8 = (v_6 << v_7);
  uint v_9 = mix(0u, 1u, (v_8 == 0u));
  uint res = ((v_1 | (v_3 | (v_5 | (v_7 | (mix(0u, 1u, (v_8 <= 2147483647u)) | v_9))))) + v_9);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_10 = VertexOutput(vec4(0.0f), 0u);
  v_10.pos = vec4(0.0f);
  v_10.prevent_dce = countLeadingZeros_208d46();
  return v_10;
}
void main() {
  VertexOutput v_11 = vertex_main_inner();
  gl_Position = vec4(v_11.pos.x, -(v_11.pos.y), ((2.0f * v_11.pos.z) - v_11.pos.w), v_11.pos.w);
  tint_interstage_location0 = v_11.prevent_dce;
  gl_PointSize = 1.0f;
}
