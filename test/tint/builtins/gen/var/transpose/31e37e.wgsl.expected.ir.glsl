#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int transpose_31e37e() {
  mat4x2 arg_0 = mat4x2(vec2(1.0f), vec2(1.0f), vec2(1.0f), vec2(1.0f));
  mat2x4 res = transpose(arg_0);
  return mix(0, 1, (res[0].x == 0.0f));
}
void main() {
  v.tint_symbol = transpose_31e37e();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  int tint_symbol;
} v;
int transpose_31e37e() {
  mat4x2 arg_0 = mat4x2(vec2(1.0f), vec2(1.0f), vec2(1.0f), vec2(1.0f));
  mat2x4 res = transpose(arg_0);
  return mix(0, 1, (res[0].x == 0.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = transpose_31e37e();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  int prevent_dce;
};

layout(location = 0) flat out int vertex_main_loc0_Output;
int transpose_31e37e() {
  mat4x2 arg_0 = mat4x2(vec2(1.0f), vec2(1.0f), vec2(1.0f), vec2(1.0f));
  mat2x4 res = transpose(arg_0);
  return mix(0, 1, (res[0].x == 0.0f));
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = transpose_31e37e();
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
