//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  vec4 inner;
} v;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2D f_arg_0;
vec4 textureLoad_6d376a() {
  uint v_2 = min(1u, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_3 = ivec2(uvec2(min(1u, (uvec2(textureSize(f_arg_0, int(v_2))).x - 1u)), 0u));
  vec4 res = texelFetch(f_arg_0, v_3, int(v_2));
  return res;
}
void main() {
  v.inner = textureLoad_6d376a();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2D arg_0;
vec4 textureLoad_6d376a() {
  uint v_2 = min(1u, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_3 = ivec2(uvec2(min(1u, (uvec2(textureSize(arg_0, int(v_2))).x - 1u)), 0u));
  vec4 res = texelFetch(arg_0, v_3, int(v_2));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_6d376a();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, std140)
uniform v_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2D v_arg_0;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec4 textureLoad_6d376a() {
  uint v_1 = min(1u, (v.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_2 = ivec2(uvec2(min(1u, (uvec2(textureSize(v_arg_0, int(v_1))).x - 1u)), 0u));
  vec4 res = texelFetch(v_arg_0, v_2, int(v_1));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_3 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_3.pos = vec4(0.0f);
  v_3.prevent_dce = textureLoad_6d376a();
  return v_3;
}
void main() {
  VertexOutput v_4 = vertex_main_inner();
  gl_Position = vec4(v_4.pos.x, -(v_4.pos.y), ((2.0f * v_4.pos.z) - v_4.pos.w), v_4.pos.w);
  tint_interstage_location0 = v_4.prevent_dce;
  gl_PointSize = 1.0f;
}
