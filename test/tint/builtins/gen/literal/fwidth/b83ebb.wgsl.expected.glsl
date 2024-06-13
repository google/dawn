#version 310 es
precision highp float;
precision highp int;

vec2 fwidth_b83ebb() {
  vec2 res = fwidth(vec2(1.0f));
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

void fragment_main() {
  prevent_dce.inner = fwidth_b83ebb();
}

void main() {
  fragment_main();
  return;
}
