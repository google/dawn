#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

vec4 dpdxFine_8c5069() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdx(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdxFine_8c5069();
}

void main() {
  fragment_main();
  return;
}
