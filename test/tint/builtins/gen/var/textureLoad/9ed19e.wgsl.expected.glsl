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
uniform highp sampler2D f_arg_0;
float textureLoad_9ed19e() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uvec2 v_2 = arg_1;
  uint v_3 = min(arg_2, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_4 = ivec2(min(v_2, (uvec2(textureSize(f_arg_0, int(v_3))) - uvec2(1u))));
  float res = texelFetch(f_arg_0, v_4, int(v_3)).x;
  return res;
}
void main() {
  v.inner = textureLoad_9ed19e();
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
uniform highp sampler2D arg_0;
float textureLoad_9ed19e() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uvec2 v_2 = arg_1;
  uint v_3 = min(arg_2, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_4 = ivec2(min(v_2, (uvec2(textureSize(arg_0, int(v_3))) - uvec2(1u))));
  float res = texelFetch(arg_0, v_4, int(v_3)).x;
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_9ed19e();
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
uniform highp sampler2D v_arg_0;
layout(location = 0) flat out float tint_interstage_location0;
float textureLoad_9ed19e() {
  uvec2 arg_1 = uvec2(1u);
  uint arg_2 = 1u;
  uvec2 v_1 = arg_1;
  uint v_2 = min(arg_2, (v.metadata[(0u / 4u)][(0u % 4u)] - 1u));
  ivec2 v_3 = ivec2(min(v_1, (uvec2(textureSize(v_arg_0, int(v_2))) - uvec2(1u))));
  float res = texelFetch(v_arg_0, v_3, int(v_2)).x;
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_4 = VertexOutput(vec4(0.0f), 0.0f);
  v_4.pos = vec4(0.0f);
  v_4.prevent_dce = textureLoad_9ed19e();
  return v_4;
}
void main() {
  VertexOutput v_5 = vertex_main_inner();
  gl_Position = vec4(v_5.pos.x, -(v_5.pos.y), ((2.0f * v_5.pos.z) - v_5.pos.w), v_5.pos.w);
  tint_interstage_location0 = v_5.prevent_dce;
  gl_PointSize = 1.0f;
}
