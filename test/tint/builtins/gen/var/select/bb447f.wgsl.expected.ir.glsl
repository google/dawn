#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec2 tint_symbol;
} v;
ivec2 select_bb447f() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  bool arg_2 = true;
  ivec2 v_1 = arg_0;
  ivec2 v_2 = arg_1;
  ivec2 res = mix(v_1, v_2, bvec2(arg_2));
  return res;
}
void main() {
  v.tint_symbol = select_bb447f();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec2 tint_symbol;
} v;
ivec2 select_bb447f() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  bool arg_2 = true;
  ivec2 v_1 = arg_0;
  ivec2 v_2 = arg_1;
  ivec2 res = mix(v_1, v_2, bvec2(arg_2));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = select_bb447f();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec2 prevent_dce;
};

layout(location = 0) flat out ivec2 vertex_main_loc0_Output;
ivec2 select_bb447f() {
  ivec2 arg_0 = ivec2(1);
  ivec2 arg_1 = ivec2(1);
  bool arg_2 = true;
  ivec2 v = arg_0;
  ivec2 v_1 = arg_1;
  ivec2 res = mix(v, v_1, bvec2(arg_2));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec2(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = select_bb447f();
  return tint_symbol;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = v_2.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
