#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uniform highp isampler3D arg_0;
uvec3 textureDimensions_c871f3() {
  uint arg_1 = 1u;
  uvec3 res = uvec3(textureSize(arg_0, int(arg_1)));
  return res;
}
void main() {
  v.tint_symbol = textureDimensions_c871f3();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec3 tint_symbol;
} v;
uniform highp isampler3D arg_0;
uvec3 textureDimensions_c871f3() {
  uint arg_1 = 1u;
  uvec3 res = uvec3(textureSize(arg_0, int(arg_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureDimensions_c871f3();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

uniform highp isampler3D arg_0;
layout(location = 0) flat out uvec3 vertex_main_loc0_Output;
uvec3 textureDimensions_c871f3() {
  uint arg_1 = 1u;
  uvec3 res = uvec3(textureSize(arg_0, int(arg_1)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureDimensions_c871f3();
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
