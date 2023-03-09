#version 310 es
precision highp float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSample_7e9ffd() {
  float res = texture(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1)), 0.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_7e9ffd();
}

void main() {
  fragment_main();
  return;
}
