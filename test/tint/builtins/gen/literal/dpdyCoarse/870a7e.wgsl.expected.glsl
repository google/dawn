#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

void dpdyCoarse_870a7e() {
  float res = dFdy(1.0f);
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdyCoarse_870a7e();
}

void main() {
  fragment_main();
  return;
}
