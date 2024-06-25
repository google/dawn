#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  vec4 inner;
} prevent_dce;

vec4 dpdyFine_d0a648() {
  vec4 res = dFdy(vec4(1.0f));
  return res;
}

void fragment_main() {
  prevent_dce.inner = dpdyFine_d0a648();
}

void main() {
  fragment_main();
  return;
}
