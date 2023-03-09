#version 310 es
precision highp float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSample_1a4e1b() {
  float res = texture(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1u)), 0.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSample_1a4e1b();
}

void main() {
  fragment_main();
  return;
}
