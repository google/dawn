//
// fragment_main
//
#version 460
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec3 inner;
} v;
layout(binding = 0, rg32f) uniform highp readonly image3D arg_0;
uvec3 textureDimensions_6dae40() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}
void main() {
  v.inner = textureDimensions_6dae40();
}
//
// compute_main
//
#version 460

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec3 inner;
} v;
layout(binding = 0, rg32f) uniform highp readonly image3D arg_0;
uvec3 textureDimensions_6dae40() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_6dae40();
}
//
// vertex_main
//
#version 460


struct VertexOutput {
  vec4 pos;
  uvec3 prevent_dce;
};

layout(binding = 0, rg32f) uniform highp readonly image3D arg_0;
layout(location = 0) flat out uvec3 tint_interstage_location0;
uvec3 textureDimensions_6dae40() {
  uvec3 res = uvec3(imageSize(arg_0));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f), uvec3(0u));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureDimensions_6dae40();
  return tint_symbol;
}
void main() {
  VertexOutput v = vertex_main_inner();
  gl_Position = v.pos;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  tint_interstage_location0 = v.prevent_dce;
  gl_PointSize = 1.0f;
}
