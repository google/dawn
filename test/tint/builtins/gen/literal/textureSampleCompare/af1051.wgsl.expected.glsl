#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

uniform highp sampler2DArrayShadow arg_0_arg_1;

float textureSampleCompare_af1051() {
  float res = textureGradOffset(arg_0_arg_1, vec4(vec3(vec2(1.0f), float(1)), 1.0f), dFdx(vec2(1.0f)), dFdy(vec2(1.0f)), ivec2(1));
  return res;
}

void fragment_main() {
  prevent_dce.inner = textureSampleCompare_af1051();
}

void main() {
  fragment_main();
  return;
}
