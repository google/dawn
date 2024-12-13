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
uint pack4x8unorm_95c456() {
  uint res = 4294967295u;
  return res;
}
void main() {
  v.inner = pack4x8unorm_95c456();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uint inner;
} v;
uint pack4x8unorm_95c456() {
  uint res = 4294967295u;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = pack4x8unorm_95c456();
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
uint pack4x8unorm_95c456() {
  uint res = 4294967295u;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0u);
  v.pos = vec4(0.0f);
  v.prevent_dce = pack4x8unorm_95c456();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
