#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

void dpdyCoarse_3e1ab4() {
  vec2 res = dFdy(vec2(1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdyCoarse_3e1ab4();
}

void main() {
  fragment_main();
  return;
}
