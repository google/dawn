#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void dpdy_699a05() {
  vec4 res = dFdy(vec4(1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdy_699a05();
}

void main() {
  fragment_main();
  return;
}
