#version 310 es
precision highp float;
precision highp int;

vec3 dpdy_feb40f() {
  vec3 res = dFdy(vec3(1.0f));
  return res;
}

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec3 inner;
  uint pad;
} prevent_dce;

void fragment_main() {
  prevent_dce.inner = dpdy_feb40f();
}

void main() {
  fragment_main();
  return;
}
