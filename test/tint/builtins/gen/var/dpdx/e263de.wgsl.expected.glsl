#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void dpdx_e263de() {
  float arg_0 = 1.0f;
  float res = dFdx(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdx_e263de();
}

void main() {
  fragment_main();
  return;
}
