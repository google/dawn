#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
uniform highp isamplerCubeArray arg_1_arg_2;
ivec4 textureGather_04fa78() {
  vec4 v_1 = vec4(vec3(1.0f), float(1u));
  ivec4 res = textureGather(arg_1_arg_2, v_1, int(1u));
  return res;
}
void main() {
  v.tint_symbol = textureGather_04fa78();
}
#version 460

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  ivec4 tint_symbol;
} v;
uniform highp isamplerCubeArray arg_1_arg_2;
ivec4 textureGather_04fa78() {
  vec4 v_1 = vec4(vec3(1.0f), float(1u));
  ivec4 res = textureGather(arg_1_arg_2, v_1, int(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureGather_04fa78();
}
#version 460


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

uniform highp isamplerCubeArray arg_1_arg_2;
layout(location = 0) flat out ivec4 vertex_main_loc0_Output;
ivec4 textureGather_04fa78() {
  vec4 v = vec4(vec3(1.0f), float(1u));
  ivec4 res = textureGather(arg_1_arg_2, v, int(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), ivec4(0));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureGather_04fa78();
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
