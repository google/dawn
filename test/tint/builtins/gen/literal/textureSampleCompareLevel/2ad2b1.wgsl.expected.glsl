#version 310 es

uniform highp sampler2DShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleCompareLevel_2ad2b1() {
  float res = texture(arg_0_arg_1, vec3(vec2(1.0f), 1.0f));
  prevent_dce.inner = res;
}

vec4 vertex_main() {
  textureSampleCompareLevel_2ad2b1();
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

uniform highp sampler2DShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleCompareLevel_2ad2b1() {
  float res = texture(arg_0_arg_1, vec3(vec2(1.0f), 1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleCompareLevel_2ad2b1();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

uniform highp sampler2DShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleCompareLevel_2ad2b1() {
  float res = texture(arg_0_arg_1, vec3(vec2(1.0f), 1.0f));
  prevent_dce.inner = res;
}

void compute_main() {
  textureSampleCompareLevel_2ad2b1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
