#version 310 es
precision mediump float;

void main_1() {
  vec2 x_1 = vec2(50.0f, 60.0f);
  vec2 x_2 = dFdy(x_1);
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
