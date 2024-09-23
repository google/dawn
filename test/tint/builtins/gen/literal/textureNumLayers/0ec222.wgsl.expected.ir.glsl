#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uniform highp sampler2DArray arg_0;
uint textureNumLayers_0ec222() {
  uint res = uint(textureSize(arg_0, 0).z);
  return res;
}
void main() {
  v.tint_symbol = textureNumLayers_0ec222();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
uniform highp sampler2DArray arg_0;
uint textureNumLayers_0ec222() {
  uint res = uint(textureSize(arg_0, 0).z);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureNumLayers_0ec222();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uint prevent_dce;
};

uniform highp sampler2DArray arg_0;
layout(location = 0) flat out uint vertex_main_loc0_Output;
uint textureNumLayers_0ec222() {
  uint res = uint(textureSize(arg_0, 0).z);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), 0u);
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureNumLayers_0ec222();
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
