#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp sampler2D arg_0_arg_1;

vec4 textureSampleBias_a161cf() {
  vec4 res = textureOffset(arg_0_arg_1, vec2(1.0f), ivec2(1), 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSampleBias_a161cf();
}

void main() {
  fragment_main();
  return;
}
