#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

vec4 dpdx_c487fa() {
  vec4 arg_0 = vec4(1.0f);
  vec4 res = dFdx(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdx_c487fa();
}

void main() {
  fragment_main();
  return;
}
