#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
uniform highp sampler2D arg_0_arg_1;
vec4 textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec2 v_1 = arg_2;
  vec2 v_2 = (vec2(0.5f) / vec2(uvec2(textureSize(arg_0_arg_1, 0))));
  vec2 v_3 = clamp(v_1, v_2, (vec2(1.0f) - v_2));
  vec4 res = textureLod(arg_0_arg_1, v_3, float(0.0f));
  return res;
}
void main() {
  v.tint_symbol = textureSampleBaseClampToEdge_9ca02c();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
uniform highp sampler2D arg_0_arg_1;
vec4 textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec2 v_1 = arg_2;
  vec2 v_2 = (vec2(0.5f) / vec2(uvec2(textureSize(arg_0_arg_1, 0))));
  vec2 v_3 = clamp(v_1, v_2, (vec2(1.0f) - v_2));
  vec4 res = textureLod(arg_0_arg_1, v_3, float(0.0f));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureSampleBaseClampToEdge_9ca02c();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

uniform highp sampler2D arg_0_arg_1;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec2 v = arg_2;
  vec2 v_1 = (vec2(0.5f) / vec2(uvec2(textureSize(arg_0_arg_1, 0))));
  vec2 v_2 = clamp(v, v_1, (vec2(1.0f) - v_1));
  vec4 res = textureLod(arg_0_arg_1, v_2, float(0.0f));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureSampleBaseClampToEdge_9ca02c();
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
