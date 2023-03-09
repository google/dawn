#version 310 es
precision highp float;

uniform highp sampler3D arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSampleBias_d3fa1b() {
  vec4 res = texture(arg_0_arg_1, vec3(1.0f), 1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleBias_d3fa1b();
}

void main() {
  fragment_main();
  return;
}
