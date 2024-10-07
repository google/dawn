#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
uniform highp usamplerCube arg_1_arg_2;
uvec4 textureGather_3b32cc() {
  vec3 arg_3 = vec3(1.0f);
  vec3 v_1 = arg_3;
  uvec4 res = textureGather(arg_1_arg_2, v_1, int(1));
  return res;
}
void main() {
  v.tint_symbol = textureGather_3b32cc();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
uniform highp usamplerCube arg_1_arg_2;
uvec4 textureGather_3b32cc() {
  vec3 arg_3 = vec3(1.0f);
  vec3 v_1 = arg_3;
  uvec4 res = textureGather(arg_1_arg_2, v_1, int(1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureGather_3b32cc();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

uniform highp usamplerCube arg_1_arg_2;
layout(location = 0) flat out uvec4 vertex_main_loc0_Output;
uvec4 textureGather_3b32cc() {
  vec3 arg_3 = vec3(1.0f);
  vec3 v = arg_3;
  uvec4 res = textureGather(arg_1_arg_2, v, int(1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureGather_3b32cc();
  return tint_symbol;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = v_1.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
