#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

float dpdxFine_f401a2() {
  float res = dFdx(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdxFine_f401a2();
}

void main() {
  fragment_main();
  return;
}
