#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void dpdyFine_6eb673() {
  float res = dFdy(1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdyFine_6eb673();
}

void main() {
  fragment_main();
  return;
}
