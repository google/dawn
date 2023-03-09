#version 310 es
precision highp float;

uniform highp sampler2D arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void textureSample_6e64fb() {
  float arg_2 = 1.0f;
  vec4 res = texture(arg_0_arg_1, vec2(arg_2, 0.5f));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_6e64fb();
}

void main() {
  fragment_main();
  return;
}
