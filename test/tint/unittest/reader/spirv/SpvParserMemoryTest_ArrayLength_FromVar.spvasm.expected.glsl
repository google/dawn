#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer S_1 {
  uint first;
  uint rtarr[];
} myvar;
void main_1() {
  uint x_1 = uint(myvar.rtarr.length());
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
