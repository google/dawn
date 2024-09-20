#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
layout(binding = 0, rgba16ui) uniform highp readonly uimage2DArray arg_0;
uvec4 textureLoad_127e12() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  int v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  uvec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
void main() {
  v.tint_symbol = textureLoad_127e12();
}
#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uvec4 tint_symbol;
} v;
layout(binding = 0, rgba16ui) uniform highp readonly uimage2DArray arg_0;
uvec4 textureLoad_127e12() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  int v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  uvec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.tint_symbol = textureLoad_127e12();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(binding = 0, rgba16ui) uniform highp readonly uimage2DArray arg_0;
layout(location = 0) flat out uvec4 vertex_main_loc0_Output;
uvec4 textureLoad_127e12() {
  ivec2 arg_1 = ivec2(1);
  int arg_2 = 1;
  int v = arg_2;
  ivec2 v_1 = ivec2(arg_1);
  uvec4 res = imageLoad(arg_0, ivec3(v_1, int(v)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_127e12();
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
