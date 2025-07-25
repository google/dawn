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
layout(binding = 1, rgba32ui) uniform highp readonly uimage3D f_arg_0;
uvec4 textureLoad_5f4473() {
  uvec4 res = imageLoad(f_arg_0, ivec3(min(uvec3(1u), (uvec3(imageSize(f_arg_0)) - uvec3(1u)))));
  return res;
}
void main() {
  v.inner = textureLoad_5f4473();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec4 inner;
} v;
layout(binding = 1, rgba32ui) uniform highp readonly uimage3D arg_0;
uvec4 textureLoad_5f4473() {
  uvec4 res = imageLoad(arg_0, ivec3(min(uvec3(1u), (uvec3(imageSize(arg_0)) - uvec3(1u)))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_5f4473();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec4 prevent_dce;
};

layout(binding = 0, rgba32ui) uniform highp readonly uimage3D v_arg_0;
layout(location = 0) flat out uvec4 tint_interstage_location0;
uvec4 textureLoad_5f4473() {
  uvec4 res = imageLoad(v_arg_0, ivec3(min(uvec3(1u), (uvec3(imageSize(v_arg_0)) - uvec3(1u)))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v = VertexOutput(vec4(0.0f), uvec4(0u));
  v.pos = vec4(0.0f);
  v.prevent_dce = textureLoad_5f4473();
  return v;
}
void main() {
  VertexOutput v_1 = vertex_main_inner();
  gl_Position = vec4(v_1.pos.x, -(v_1.pos.y), ((2.0f * v_1.pos.z) - v_1.pos.w), v_1.pos.w);
  tint_interstage_location0 = v_1.prevent_dce;
  gl_PointSize = 1.0f;
}
