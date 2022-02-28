#version 310 es
precision mediump float;

void main_1() {
  float x_2 = dFdy(50.0f);
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
