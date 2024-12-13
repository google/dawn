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

layout(location = 0) flat out vec3 vertex_main_loc0_Output;
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
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec3(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = smoothstep_aad1db();
  return tint_symbol;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = v_2.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
