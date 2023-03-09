#version 310 es
precision highp float;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

void dpdxFine_8c5069() {
  vec4 res = dFdx(vec4(1.0f));
  prevent_dce.inner = res;
}

void fragment_main() {
  dpdxFine_8c5069();
}

void main() {
  fragment_main();
  return;
}
