#version 310 es
precision highp float;
precision highp int;


vec4 tint_textureSampleBaseClampToEdge(highp sampler2D t_s, vec2 coord) {
  vec2 dims = vec2(uvec2(textureSize(t_s, 0)));
  vec2 half_texel = (vec2(0.5f) / dims);
  vec2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return textureLod(t_s, clamped, 0.0f);
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp sampler2D arg_0_arg_1;
vec4 textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = tint_textureSampleBaseClampToEdge(arg_0_arg_1, arg_2);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

void fragment_main() {
  prevent_dce.inner = textureSampleBaseClampToEdge_9ca02c();
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

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp sampler2D arg_0_arg_1;
vec4 textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = tint_textureSampleBaseClampToEdge(arg_0_arg_1, arg_2);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

void compute_main() {
  prevent_dce.inner = textureSampleBaseClampToEdge_9ca02c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
#version 310 es


vec4 tint_textureSampleBaseClampToEdge(highp sampler2D t_s, vec2 coord) {
  vec2 dims = vec2(uvec2(textureSize(t_s, 0)));
  vec2 half_texel = (vec2(0.5f) / dims);
  vec2 clamped = clamp(coord, half_texel, (1.0f - half_texel));
  return textureLod(t_s, clamped, 0.0f);
}

layout(location = 0) flat out vec4 prevent_dce_1;
uniform highp sampler2D arg_0_arg_1;
vec4 textureSampleBaseClampToEdge_9ca02c() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = tint_textureSampleBaseClampToEdge(arg_0_arg_1, arg_2);
  return res;
}

struct VertexOutput {
  vec4 pos;
  vec4 prevent_dce;
};

VertexOutput vertex_main() {
  VertexOutput tint_symbol = VertexOutput(vec4(0.0f, 0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 0.0f));
  tint_symbol.pos = vec4(0.0f);
  tint_symbol.prevent_dce = textureSampleBaseClampToEdge_9ca02c();
  return tint_symbol;
}

void main() {
  gl_PointSize = 1.0;
  VertexOutput inner_result = vertex_main();
  gl_Position = inner_result.pos;
  prevent_dce_1 = inner_result.prevent_dce;
  gl_Position.y = -(gl_Position.y);
  gl_Position.z = ((2.0f * gl_Position.z) - gl_Position.w);
  return;
}
