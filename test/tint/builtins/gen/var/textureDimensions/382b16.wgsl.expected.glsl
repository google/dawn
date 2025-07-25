//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430)
buffer f_prevent_dce_block_ssbo {
  uvec2 inner;
} v;
layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp samplerCube f_arg_0;
uvec2 textureDimensions_382b16() {
  uint arg_1 = 1u;
  uvec2 res = uvec2(textureSize(f_arg_0, int(min(arg_1, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u)))));
  return res;
}
void main() {
  v.inner = textureDimensions_382b16();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std430)
buffer prevent_dce_block_1_ssbo {
  uvec2 inner;
} v;
layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uvec4 metadata[1];
} v_1;
uniform highp samplerCube arg_0;
uvec2 textureDimensions_382b16() {
  uint arg_1 = 1u;
  uvec2 res = uvec2(textureSize(arg_0, int(min(arg_1, (v_1.metadata[(0u / 4u)][(0u % 4u)] - 1u)))));
  return res;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  v.inner = textureDimensions_382b16();
}
//
// vertex_main
//
#version 310 es


struct VertexOutput {
  vec4 pos;
  uvec2 prevent_dce;
};

layout(binding = 0, std140)
uniform v_TintTextureUniformData_ubo {
  uvec4 metadata[1];
} v;
uniform highp samplerCube v_arg_0;
layout(location = 0) flat out uvec2 tint_interstage_location0;
uvec2 textureDimensions_382b16() {
  uint arg_1 = 1u;
  uvec2 res = uvec2(textureSize(v_arg_0, int(min(arg_1, (v.metadata[(0u / 4u)][(0u % 4u)] - 1u)))));
  return res;
}
VertexOutput vertex_main_inner() {
  VertexOutput v_1 = VertexOutput(vec4(0.0f), uvec2(0u));
  v_1.pos = vec4(0.0f);
  v_1.prevent_dce = textureDimensions_382b16();
  return v_1;
}
void main() {
  VertexOutput v_2 = vertex_main_inner();
  gl_Position = vec4(v_2.pos.x, -(v_2.pos.y), ((2.0f * v_2.pos.z) - v_2.pos.w), v_2.pos.w);
  tint_interstage_location0 = v_2.prevent_dce;
  gl_PointSize = 1.0f;
}
