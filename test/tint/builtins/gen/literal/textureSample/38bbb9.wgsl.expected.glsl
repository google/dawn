#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

uniform highp sampler2DShadow arg_0_arg_1;

float textureSample_38bbb9() {
  float res = texture(arg_0_arg_1, vec3(vec2(1.0f), 0.0f));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSample_38bbb9();
}

void main() {
  fragment_main();
  return;
}
