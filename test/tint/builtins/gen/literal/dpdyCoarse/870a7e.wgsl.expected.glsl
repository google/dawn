#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

float dpdyCoarse_870a7e() {
  float res = dFdy(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdyCoarse_870a7e();
}

void main() {
  fragment_main();
  return;
}
