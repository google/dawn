#version 310 es
#extension GL_EXT_texture_shadow_lod: require
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleLevel_36f0d3() {
  vec4 v_1 = vec4(vec2(1.0f), float(1), 0.0f);
  float res = textureLodOffset(arg_0_arg_1, v_1, float(1u), ivec2(1));
  return res;
}
void main() {
  v.tint_symbol = textureSampleLevel_36f0d3();
}
#version 310 es
#extension GL_EXT_texture_shadow_lod: require

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  float tint_symbol;
} v;
uniform highp sampler2DArrayShadow arg_0_arg_1;
float textureSampleLevel_36f0d3() {
  vec4 v_1 = vec4(vec2(1.0f), float(1), 0.0f);
  float res = textureLodOffset(arg_0_arg_1, v_1, float(1u), ivec2(1));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureSampleLevel_36f0d3();
}
#version 310 es
#extension GL_EXT_texture_shadow_lod: require


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

uniform highp sampler2DArrayShadow arg_0_arg_1;
layout(location = 0) flat out float vertex_main_loc0_Output;
float textureSampleLevel_36f0d3() {
  vec4 v = vec4(vec2(1.0f), float(1), 0.0f);
  float res = textureLodOffset(arg_0_arg_1, v, float(1u), ivec2(1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0.0f);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureSampleLevel_36f0d3();
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
