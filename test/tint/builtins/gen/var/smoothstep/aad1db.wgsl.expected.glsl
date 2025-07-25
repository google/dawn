//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec3 inner;
} v;
vec3 smoothstep_aad1db() {
  vec3 arg_0 = vec3(2.0f);
  vec3 arg_1 = vec3(4.0f);
  vec3 arg_2 = vec3(3.0f);
  vec3 v_1 = arg_0;
  vec3 v_2 = clamp(((arg_2 - v_1) / (arg_1 - v_1)), vec3(0.0f), vec3(1.0f));
  vec3 res = (v_2 * (v_2 * (vec3(3.0f) - (vec3(2.0f) * v_2))));
  return res;
}
void main() {
  v.inner = smoothstep_aad1db();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec3 inner;
} v;
vec3 smoothstep_aad1db() {
  vec3 arg_0 = vec3(2.0f);
  vec3 arg_1 = vec3(4.0f);
  vec3 arg_2 = vec3(3.0f);
  vec3 v_1 = arg_0;
  vec3 v_2 = clamp(((arg_2 - v_1) / (arg_1 - v_1)), vec3(0.0f), vec3(1.0f));
  vec3 res = (v_2 * (v_2 * (vec3(3.0f) - (vec3(2.0f) * v_2))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = smoothstep_aad1db();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec3 prevent_dce;
};

layout(location = 0) flat out vec3 tint_interstage_location0;
vec3 smoothstep_aad1db() {
  vec3 arg_0 = vec3(2.0f);
  vec3 arg_1 = vec3(4.0f);
  vec3 arg_2 = vec3(3.0f);
  vec3 v = arg_0;
  vec3 v_1 = clamp(((arg_2 - v) / (arg_1 - v)), vec3(0.0f), vec3(1.0f));
  vec3 res = (v_1 * (v_1 * (vec3(3.0f) - (vec3(2.0f) * v_1))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_2 = VertexOutput(vec4(0.0f), vec3(0.0f));
  v_2.pos = vec4(0.0f);
  v_2.prevent_dce = smoothstep_aad1db();
  return v_2;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = vec4(v_3.pos.x, -(v_3.pos.y), ((2.0f * v_3.pos.z) - v_3.pos.w), v_3.pos.w);
  tint_interstage_location0 = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
