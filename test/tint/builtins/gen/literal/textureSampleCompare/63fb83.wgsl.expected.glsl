#version 310 es
precision highp float;

uniform highp samplerCubeShadow arg_0_arg_1;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void textureSampleCompare_63fb83() {
  float res = texture(arg_0_arg_1, vec4(vec3(1.0f), 1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  textureSampleCompare_63fb83();
}

void main() {
  fragment_main();
  return;
}
