//
// vertex_main
//
#version 310 es

layout(binding = 0, std140)
uniform v_TintTextureUniformData_ubo {
  uint tint_builtin_value_0;
} v;
uniform highp isampler2D v_arg_0;
ivec4 textureLoad2d(ivec2 coords, int level) {
  uint v_1 = (v.tint_builtin_value_0 - 1u);
  uint v_2 = min(uint(level), v_1);
  uvec2 v_3 = (uvec2(textureSize(v_arg_0, int(v_2))) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(coords), v_3));
  return texelFetch(v_arg_0, v_4, int(v_2));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0);
}
vec4 vertex_main_inner() {
  doTextureLoad();
  return vec4(0.0f);
}
void main() {
  vec4 v_5 = vertex_main_inner();
  gl_Position = vec4(v_5.x, -(v_5.y), ((2.0f * v_5.z) - v_5.w), v_5.w);
  gl_PointSize = 1.0f;
}
//
// fragment_main
//
#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std140)
uniform f_TintTextureUniformData_ubo {
  uint tint_builtin_value_0;
} v;
uniform highp isampler2D f_arg_0;
ivec4 textureLoad2d(ivec2 coords, int level) {
  uint v_1 = (v.tint_builtin_value_0 - 1u);
  uint v_2 = min(uint(level), v_1);
  uvec2 v_3 = (uvec2(textureSize(f_arg_0, int(v_2))) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(coords), v_3));
  return texelFetch(f_arg_0, v_4, int(v_2));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0);
}
void main() {
  doTextureLoad();
}
//
// compute_main
//
#version 310 es

layout(binding = 0, std140)
uniform TintTextureUniformData_1_ubo {
  uint tint_builtin_value_0;
} v;
uniform highp isampler2D arg_0;
ivec4 textureLoad2d(ivec2 coords, int level) {
  uint v_1 = (v.tint_builtin_value_0 - 1u);
  uint v_2 = min(uint(level), v_1);
  uvec2 v_3 = (uvec2(textureSize(arg_0, int(v_2))) - uvec2(1u));
  ivec2 v_4 = ivec2(min(uvec2(coords), v_3));
  return texelFetch(arg_0, v_4, int(v_2));
}
void doTextureLoad() {
  ivec4 res = textureLoad2d(ivec2(0), 0);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  doTextureLoad();
}
