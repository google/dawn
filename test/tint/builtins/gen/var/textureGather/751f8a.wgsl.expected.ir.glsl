#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp samplerCubeArray arg_1_arg_2;
vec4 textureGather_751f8a() {
  vec3 arg_3 = vec3(1.0f);
  int arg_4 = 1;
  vec3 v_1 = arg_3;
  vec4 v_2 = vec4(v_1, float(arg_4));
  vec4 res = textureGather(arg_1_arg_2, v_2, int(1));
  return res;
}
void main() {
  v.inner = textureGather_751f8a();
}
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
uniform highp samplerCubeArray arg_1_arg_2;
vec4 textureGather_751f8a() {
  vec3 arg_3 = vec3(1.0f);
  int arg_4 = 1;
  vec3 v_1 = arg_3;
  vec4 v_2 = vec4(v_1, float(arg_4));
  vec4 res = textureGather(arg_1_arg_2, v_2, int(1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureGather_751f8a();
}
#version 460


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

uniform highp samplerCubeArray arg_1_arg_2;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 textureGather_751f8a() {
  vec3 arg_3 = vec3(1.0f);
  int arg_4 = 1;
  vec3 v = arg_3;
  vec4 v_1 = vec4(v, float(arg_4));
  vec4 res = textureGather(arg_1_arg_2, v_1, int(1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureGather_751f8a();
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
