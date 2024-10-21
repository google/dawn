#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
vec4 step_e2b337() {
  vec4 arg_0 = vec4(1.0f);
  vec4 arg_1 = vec4(1.0f);
  vec4 res = step(arg_0, arg_1);
  return res;
}
void main() {
  v.inner = step_e2b337();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
vec4 step_e2b337() {
  vec4 arg_0 = vec4(1.0f);
  vec4 arg_1 = vec4(1.0f);
  vec4 res = step(arg_0, arg_1);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = step_e2b337();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 step_e2b337() {
  vec4 arg_0 = vec4(1.0f);
  vec4 arg_1 = vec4(1.0f);
  vec4 res = step(arg_0, arg_1);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = step_e2b337();
  return tint_symbol;
}
void main() {
  VertexOutput v = vertex_main_inner();
  gl_Position = v.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v.prevent_dce;
  gl_PointSize = 1.0f;
}
