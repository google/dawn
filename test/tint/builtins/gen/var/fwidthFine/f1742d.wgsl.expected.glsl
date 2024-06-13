#version 310 es
precision highp float;
precision highp int;

float fwidthFine_f1742d() {
  float arg_0 = 1.0f;
  float res = fwidth(arg_0);
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void fragment_main() {
  prevent_dce.inner = fwidthFine_f1742d();
}

void main() {
  fragment_main();
  return;
}
