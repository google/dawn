#version 310 es


vec4 tint_textureSampleBaseClampToEdge(highp sampler2D t_s, vec2 coord) {
  vec2 dims = vec2(uvec2(textureSize(t_s, 0)));
  vec2 half_texel = (vec2(0.5f) / dims);
  vec2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return textureLod(t_s, clamped, 0.0f);
}

uniform highp sampler2D arg_0_arg_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = tint_textureSampleBaseClampToEdge(arg_0_arg_1, arg_2);
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureSampleBaseClampToEdge_9ca02c();
  return vec4(0.0f);
}

void main() {
  gl_PointSize = 1.0;
  vec4 inner_result = vertex_main();
  gl_Position = inner_result;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
#version 310 es
precision highp float;


vec4 tint_textureSampleBaseClampToEdge(highp sampler2D t_s, vec2 coord) {
  vec2 dims = vec2(uvec2(textureSize(t_s, 0)));
  vec2 half_texel = (vec2(0.5f) / dims);
  vec2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return textureLod(t_s, clamped, 0.0f);
}

uniform highp sampler2D arg_0_arg_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = tint_textureSampleBaseClampToEdge(arg_0_arg_1, arg_2);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleBaseClampToEdge_9ca02c();
}

void main() {
  fragment_main();
  return;
}
#version 310 es


vec4 tint_textureSampleBaseClampToEdge(highp sampler2D t_s, vec2 coord) {
  vec2 dims = vec2(uvec2(textureSize(t_s, 0)));
  vec2 half_texel = (vec2(0.5f) / dims);
  vec2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return textureLod(t_s, clamped, 0.0f);
}

uniform highp sampler2D arg_0_arg_1;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = tint_textureSampleBaseClampToEdge(arg_0_arg_1, arg_2);
  prevent_dce.inner = res;
}

void compute_main() {
  textureSampleBaseClampToEdge_9ca02c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
