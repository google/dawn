#version 310 es
precision highp float;

uniform highp sampler2DArrayShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleCompare_90ae56() {
  vec2 arg_2 = vec2(1.0f);
  uint arg_3 = 1u;
  float arg_4 = 1.0f;
  float res = texture(arg_0_arg_1, vec4(vec3(arg_2, float(arg_3)), arg_4));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleCompare_90ae56();
}

void main() {
  fragment_main();
  return;
}
