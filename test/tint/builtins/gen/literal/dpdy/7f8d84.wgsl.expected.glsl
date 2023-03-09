#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void dpdy_7f8d84() {
  float res = dFdy(1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdy_7f8d84();
}

void main() {
  fragment_main();
  return;
}
