#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void dpdxFine_f401a2() {
  float res = dFdx(1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdxFine_f401a2();
}

void main() {
  fragment_main();
  return;
}
