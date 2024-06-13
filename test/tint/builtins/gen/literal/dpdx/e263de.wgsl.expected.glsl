#version 310 es
precision highp float;
precision highp int;

float dpdx_e263de() {
  float res = dFdx(1.0f);
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void fragment_main() {
  prevent_dce.inner = dpdx_e263de();
}

void main() {
  fragment_main();
  return;
}
