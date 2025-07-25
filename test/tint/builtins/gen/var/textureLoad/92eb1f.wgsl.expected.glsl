//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp usampler3D f_arg_0;
uvec4 textureLoad_92eb1f() {
  uvec3 arg_1 = uvec3(1u);
  int arg_2 = 1;
  uvec3 v_2 = arg_1;
  uint v_3 = (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_4 = min(uint(arg_2), v_3);
  ivec3 v_5 = ivec3(min(v_2, (uvec3(textureSize(f_arg_0, int(v_4))) - uvec3(1u))));
  uvec4 res = texelFetch(f_arg_0, v_5, int(v_4));
  return res;
}
void main() {
  v.inner = textureLoad_92eb1f();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp usampler3D arg_0;
uvec4 textureLoad_92eb1f() {
  uvec3 arg_1 = uvec3(1u);
  int arg_2 = 1;
  uvec3 v_2 = arg_1;
  uint v_3 = (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_4 = min(uint(arg_2), v_3);
  ivec3 v_5 = ivec3(min(v_2, (uvec3(textureSize(arg_0, int(v_4))) - uvec3(1u))));
  uvec4 res = texelFetch(arg_0, v_5, int(v_4));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_92eb1f();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(binding = 0, std140)
uniform v_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp usampler3D v_arg_0;
layout(location = 0) flat out uvec4 tint_interstage_location0;
uvec4 textureLoad_92eb1f() {
  uvec3 arg_1 = uvec3(1u);
  int arg_2 = 1;
  uvec3 v_1 = arg_1;
  uint v_2 = (v.metadata[(0u / 4u)][(0u % 4u)] - 1u);
  uint v_3 = min(uint(arg_2), v_2);
  ivec3 v_4 = ivec3(min(v_1, (uvec3(textureSize(v_arg_0, int(v_3))) - uvec3(1u))));
  uvec4 res = texelFetch(v_arg_0, v_4, int(v_3));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_5 = VertexOutput(vec4(0.0f), uvec4(0u));
  v_5.pos = vec4(0.0f);
  v_5.prevent_dce = textureLoad_92eb1f();
  return v_5;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = vec4(v_6.pos.x, -(v_6.pos.y), ((2.0f * v_6.pos.z) - v_6.pos.w), v_6.pos.w);
  tint_interstage_location0 = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
