#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

uniform highp sampler2D arg_0_arg_1;

vec4 textureSampleBias_6a9113() {
  vec4 res = texture(arg_0_arg_1, vec2(1.0f), 1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSampleBias_6a9113();
}

void main() {
  fragment_main();
  return;
}
