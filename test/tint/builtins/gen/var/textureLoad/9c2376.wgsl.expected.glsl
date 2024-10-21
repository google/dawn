#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r32f) uniform highp readonly image2DArray arg_0;
vec4 textureLoad_9c2376() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  uint v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  vec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
void main() {
  v.inner = textureLoad_9c2376();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, r32f) uniform highp readonly image2DArray arg_0;
vec4 textureLoad_9c2376() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  uint v_1 = arg_2;
  ivec2 v_2 = ivec2(arg_1);
  vec4 res = imageLoad(arg_0, ivec3(v_2, int(v_1)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_9c2376();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, r32f) uniform highp readonly image2DArray arg_0;
layout(location = 0) flat out vec4 vertex_main_loc0_Output;
vec4 textureLoad_9c2376() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  uint v = arg_2;
  ivec2 v_1 = ivec2(arg_1);
  vec4 res = imageLoad(arg_0, ivec3(v_1, int(v)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), vec4(0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_9c2376();
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
