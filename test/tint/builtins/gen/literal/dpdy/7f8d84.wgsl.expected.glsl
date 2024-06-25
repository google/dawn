#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

float dpdy_7f8d84() {
  float res = dFdy(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdy_7f8d84();
}

void main() {
  fragment_main();
  return;
}
