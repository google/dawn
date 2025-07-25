//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  float inner;
} v;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2DArray f_arg_0;
float textureLoad_04b911() {
  uint v_2 = (uint(textureSize(f_arg_0, 0).z) - 1u);
  uint v_3 = min(uint(1), v_2);
  uint v_4 = min(1u, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_5 = ivec2(min(uvec2(1u), (uvec2(textureSize(f_arg_0, int(v_4)).xy) - uvec2(1u))));
  ivec3 v_6 = ivec3(v_5, int(v_3));
  float res = texelFetch(f_arg_0, v_6, int(v_4)).x;
  return res;
}
void main() {
  v.inner = textureLoad_04b911();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  float inner;
} v;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp sampler2DArray arg_0;
float textureLoad_04b911() {
  uint v_2 = (uint(textureSize(arg_0, 0).z) - 1u);
  uint v_3 = min(uint(1), v_2);
  uint v_4 = min(1u, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_5 = ivec2(min(uvec2(1u), (uvec2(textureSize(arg_0, int(v_4)).xy) - uvec2(1u))));
  ivec3 v_6 = ivec3(v_5, int(v_3));
  float res = texelFetch(arg_0, v_6, int(v_4)).x;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_04b911();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  float prevent_dce;
};

layout(binding = 0, std140)
uniform v_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp sampler2DArray v_arg_0;
layout(location = 0) flat out float tint_interstage_location0;
float textureLoad_04b911() {
  uint v_1 = (uint(textureSize(v_arg_0, 0).z) - 1u);
  uint v_2 = min(uint(1), v_1);
  uint v_3 = min(1u, (v.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_4 = ivec2(min(uvec2(1u), (uvec2(textureSize(v_arg_0, int(v_3)).xy) - uvec2(1u))));
  ivec3 v_5 = ivec3(v_4, int(v_2));
  float res = texelFetch(v_arg_0, v_5, int(v_3)).x;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_6 = VertexOutput(vec4(0.0f), 0.0f);
  v_6.pos = vec4(0.0f);
  v_6.prevent_dce = textureLoad_04b911();
  return v_6;
}
void main() {
  VertexOutput v_7 = vertex_main_inner();
  gl_Position = vec4(v_7.pos.x, -(v_7.pos.y), ((2.0f * v_7.pos.z) - v_7.pos.w), v_7.pos.w);
  tint_interstage_location0 = v_7.prevent_dce;
  gl_PointSize = 1.0f;
}
