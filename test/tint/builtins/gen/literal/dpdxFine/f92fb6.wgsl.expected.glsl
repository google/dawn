#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
} prevent_dce;

vec3 dpdxFine_f92fb6() {
  vec3 res = dFdx(vec3(1.0f));
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdxFine_f92fb6();
}

void main() {
  fragment_main();
  return;
}
