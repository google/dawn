#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

float dpdx_e263de() {
  float arg_0 = 1.0f;
  float res = dFdx(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdx_e263de();
}

void main() {
  fragment_main();
  return;
}
