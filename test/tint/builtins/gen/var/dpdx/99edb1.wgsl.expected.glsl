#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec2 inner;
} prevent_dce;

void dpdx_99edb1() {
  vec2 arg_0 = vec2(1.0f);
  vec2 res = dFdx(arg_0);
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdx_99edb1();
}

void main() {
  fragment_main();
  return;
}
