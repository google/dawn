//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  ivec4 inner;
} v;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp isampler2DArray f_arg_0;
ivec4 textureLoad_9885b0() {
  uint v_2 = min(1u, (uint(textureSize(f_arg_0, 0).z) - 1u));
  uint v_3 = min(1u, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_4 = ivec2(min(uvec2(1u), (uvec2(textureSize(f_arg_0, int(v_3)).xy) - uvec2(1u))));
  ivec3 v_5 = ivec3(v_4, int(v_2));
  ivec4 res = texelFetch(f_arg_0, v_5, int(v_3));
  return res;
}
void main() {
  v.inner = textureLoad_9885b0();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  ivec4 inner;
} v;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp isampler2DArray arg_0;
ivec4 textureLoad_9885b0() {
  uint v_2 = min(1u, (uint(textureSize(arg_0, 0).z) - 1u));
  uint v_3 = min(1u, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_4 = ivec2(min(uvec2(1u), (uvec2(textureSize(arg_0, int(v_3)).xy) - uvec2(1u))));
  ivec3 v_5 = ivec3(v_4, int(v_2));
  ivec4 res = texelFetch(arg_0, v_5, int(v_3));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_9885b0();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  ivec4 prevent_dce;
};

layout(binding = 0, std140)
uniform v_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp isampler2DArray v_arg_0;
layout(location = 0) flat out ivec4 tint_interstage_location0;
ivec4 textureLoad_9885b0() {
  uint v_1 = min(1u, (uint(textureSize(v_arg_0, 0).z) - 1u));
  uint v_2 = min(1u, (v.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_3 = ivec2(min(uvec2(1u), (uvec2(textureSize(v_arg_0, int(v_2)).xy) - uvec2(1u))));
  ivec3 v_4 = ivec3(v_3, int(v_1));
  ivec4 res = texelFetch(v_arg_0, v_4, int(v_2));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_5 = VertexOutput(vec4(0.0f), ivec4(0));
  v_5.pos = vec4(0.0f);
  v_5.prevent_dce = textureLoad_9885b0();
  return v_5;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = vec4(v_6.pos.x, -(v_6.pos.y), ((2.0f * v_6.pos.z) - v_6.pos.w), v_6.pos.w);
  tint_interstage_location0 = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
