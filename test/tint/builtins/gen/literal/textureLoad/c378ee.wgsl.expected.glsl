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
uniform highp usampler2DMS f_arg_0;
uvec4 textureLoad_c378ee() {
  uvec2 v_1 = (uvec2(textureSize(f_arg_0)) - uvec2(1u));
  uvec4 res = texelFetch(f_arg_0, ivec2(min(uvec2(ivec2(1)), v_1)), 1);
  return res;
}
void main() {
  v.inner = textureLoad_c378ee();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
uniform highp usampler2DMS arg_0;
uvec4 textureLoad_c378ee() {
  uvec2 v_1 = (uvec2(textureSize(arg_0)) - uvec2(1u));
  uvec4 res = texelFetch(arg_0, ivec2(min(uvec2(ivec2(1)), v_1)), 1);
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_c378ee();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

uniform highp usampler2DMS v_arg_0;
layout(location = 0) flat out uvec4 tint_interstage_location0;
uvec4 textureLoad_c378ee() {
  uvec2 v = (uvec2(textureSize(v_arg_0)) - uvec2(1u));
  uvec4 res = texelFetch(v_arg_0, ivec2(min(uvec2(ivec2(1)), v)), 1);
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), uvec4(0u));
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = textureLoad_c378ee();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
