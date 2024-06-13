#version 310 es
precision highp float;
precision highp int;

float dpdyFine_6eb673() {
  float res = dFdy(1.0f);
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void fragment_main() {
  prevent_dce.inner = dpdyFine_6eb673();
}

void main() {
  fragment_main();
  return;
}
