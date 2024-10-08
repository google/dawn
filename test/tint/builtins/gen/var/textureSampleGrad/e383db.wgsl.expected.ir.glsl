#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
uniform highp samplerCubeArray arg_0_arg_1;
vec4 textureSampleGrad_e383db() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  vec3 arg_4 = vec3(1.0f);
  vec3 arg_5 = vec3(1.0f);
  vec3 v_1 = arg_2;
  vec3 v_2 = arg_4;
  vec3 v_3 = arg_5;
  vec4 res = textureGrad(arg_0_arg_1, vec4(v_1, float(arg_3)), v_2, v_3);
  return res;
}
void main() {
  v.tint_symbol = textureSampleGrad_e383db();
}
#version 460

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
uniform highp samplerCubeArray arg_0_arg_1;
vec4 textureSampleGrad_e383db() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  vec3 arg_4 = vec3(1.0f);
  vec3 arg_5 = vec3(1.0f);
  vec3 v_1 = arg_2;
  vec3 v_2 = arg_4;
  vec3 v_3 = arg_5;
  vec4 res = textureGrad(arg_0_arg_1, vec4(v_1, float(arg_3)), v_2, v_3);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureSampleGrad_e383db();
}
#version 460


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

uniform highp samplerCubeArray arg_0_arg_1;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 textureSampleGrad_e383db() {
  vec3 arg_2 = vec3(1.0f);
  int arg_3 = 1;
  vec3 arg_4 = vec3(1.0f);
  vec3 arg_5 = vec3(1.0f);
  vec3 v = arg_2;
  vec3 v_1 = arg_4;
  vec3 v_2 = arg_5;
  vec4 res = textureGrad(arg_0_arg_1, vec4(v, float(arg_3)), v_1, v_2);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureSampleGrad_e383db();
  return tint_symbol;
}
void main() {
  VertexOutput v_3 = vertex_main_inner();
  gl_Position = v_3.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_3.prevent_dce;
  gl_PointSize = 1.0f;
}
