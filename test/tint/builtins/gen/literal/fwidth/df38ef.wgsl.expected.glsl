#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  float inner;
} prevent_dce;

float fwidth_df38ef() {
  float res = fwidth(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.inner = fwidth_df38ef();
}

void main() {
  fragment_main();
  return;
}
