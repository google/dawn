#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, rgba32ui) uniform highp readonly uimage2DArray arg_0;
uvec4 textureLoad_c40dcb() {
  uint v_1 = (uint(imageSize(arg_0).z) - 1u);
  uint v_2 = min(uint(1), v_1);
  uvec2 v_3 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(1)), v_3));
  uvec4 res = imageLoad(arg_0, ivec3(v_4, int(v_2)));
  return res;
}
void main() {
  v.inner = textureLoad_c40dcb();
}
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, rgba32ui) uniform highp readonly uimage2DArray arg_0;
uvec4 textureLoad_c40dcb() {
  uint v_1 = (uint(imageSize(arg_0).z) - 1u);
  uint v_2 = min(uint(1), v_1);
  uvec2 v_3 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(ivec2(1)), v_3));
  uvec4 res = imageLoad(arg_0, ivec3(v_4, int(v_2)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_c40dcb();
}
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(binding = 0, rgba32ui) uniform highp readonly uimage2DArray arg_0;
layout(location = 0) flat out uvec4 vertex_main_loc0_Output;
uvec4 textureLoad_c40dcb() {
  uint v = (uint(imageSize(arg_0).z) - 1u);
  uint v_1 = min(uint(1), v);
  uvec2 v_2 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_3 = ivec2(min(uvec2(ivec2(1)), v_2));
  uvec4 res = imageLoad(arg_0, ivec3(v_3, int(v_1)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec4(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureLoad_c40dcb();
  return tint_symbol;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = v_4.pos;
  gl_Position[1u] = -(gl_Position.y);
  gl_Position[2u] = ((2.0f * gl_Position.z) - gl_Position.w);
  vertex_main_loc0_Output = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
