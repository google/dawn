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
layout(binding = 1, rgba8_snorm) uniform highp readonly image2DArray f_arg_0;
vec4 textureLoad_9de6f5() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec2 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = min(v_2, (uint(imageSize(f_arg_0).z) - 1u));
  uvec2 v_4 = (uvec2(imageSize(f_arg_0).xy) - uvec2(1u));
  ivec2 v_5 = ivec2(min(uvec2(v_1), v_4));
  vec4 res = imageLoad(f_arg_0, ivec3(v_5, int(v_3)));
  return res;
}
void main() {
  v.inner = textureLoad_9de6f5();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  vec4 inner;
} v;
layout(binding = 1, rgba8_snorm) uniform highp readonly image2DArray arg_0;
vec4 textureLoad_9de6f5() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec2 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = min(v_2, (uint(imageSize(arg_0).z) - 1u));
  uvec2 v_4 = (uvec2(imageSize(arg_0).xy) - uvec2(1u));
  ivec2 v_5 = ivec2(min(uvec2(v_1), v_4));
  vec4 res = imageLoad(arg_0, ivec3(v_5, int(v_3)));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureLoad_9de6f5();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

layout(binding = 0, rgba8_snorm) uniform highp readonly image2DArray v_arg_0;
layout(location = 0) flat out vec4 tint_interstage_location0;
vec4 textureLoad_9de6f5() {
  ivec2 arg_1 = ivec2(1);
  uint arg_2 = 1u;
  ivec2 v = arg_1;
  uint v_1 = arg_2;
  uint v_2 = min(v_1, (uint(imageSize(v_arg_0).z) - 1u));
  uvec2 v_3 = (uvec2(imageSize(v_arg_0).xy) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(v), v_3));
  vec4 res = imageLoad(v_arg_0, ivec3(v_4, int(v_2)));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_5 = VertexOutput(vec4(0.0f), vec4(0.0f));
  v_5.pos = vec4(0.0f);
  v_5.prevent_dce = textureLoad_9de6f5();
  return v_5;
}
void main() {
  VertexOutput v_6 = vertex_main_inner();
  gl_Position = vec4(v_6.pos.x, -(v_6.pos.y), ((2.0f * v_6.pos.z) - v_6.pos.w), v_6.pos.w);
  tint_interstage_location0 = v_6.prevent_dce;
  gl_PointSize = 1.0f;
}
