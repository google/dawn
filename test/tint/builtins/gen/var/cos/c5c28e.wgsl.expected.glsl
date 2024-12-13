//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
float cos_c5c28e() {
  float arg_0 = 0.0f;
  float res = cos(arg_0);
  return res;
}
void main() {
  v.inner = cos_c5c28e();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
float cos_c5c28e() {
  float arg_0 = 0.0f;
  float res = cos(arg_0);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = cos_c5c28e();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

layout(location = 0) flat out float tint_interstage_location0;
float cos_c5c28e() {
  float arg_0 = 0.0f;
  float res = cos(arg_0);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), 0.0f);
  v.pos = vec4(0.0f);
  v.prevent_dce = cos_c5c28e();
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
