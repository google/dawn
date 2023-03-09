#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void fwidthFine_f1742d() {
  float res = fwidth(1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  fwidthFine_f1742d();
}

void main() {
  fragment_main();
  return;
}
