#version 310 es
precision highp float;

uniform highp sampler2D arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSample_51b514() {
  vec2 arg_2 = vec2(1.0f);
  vec4 res = texture(arg_0_arg_1, arg_2);
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_51b514();
}

void main() {
  fragment_main();
  return;
}
