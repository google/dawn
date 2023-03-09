#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void fwidthFine_f1742d() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  fwidthFine_f1742d();
}

void main() {
  fragment_main();
  return;
}
