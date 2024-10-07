#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
uniform highp sampler2DArray arg_1_arg_2;
vec4 textureGather_ea8eb4() {
  vec3 v_1 = vec3(vec2(1.0f), float(1u));
  vec4 res = textureGather(arg_1_arg_2, v_1, int(1u));
  return res;
}
void main() {
  v.tint_symbol = textureGather_ea8eb4();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  vec4 tint_symbol;
} v;
uniform highp sampler2DArray arg_1_arg_2;
vec4 textureGather_ea8eb4() {
  vec3 v_1 = vec3(vec2(1.0f), float(1u));
  vec4 res = textureGather(arg_1_arg_2, v_1, int(1u));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureGather_ea8eb4();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

uniform highp sampler2DArray arg_1_arg_2;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 textureGather_ea8eb4() {
  vec3 v = vec3(vec2(1.0f), float(1u));
  vec4 res = textureGather(arg_1_arg_2, v, int(1u));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureGather_ea8eb4();
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
