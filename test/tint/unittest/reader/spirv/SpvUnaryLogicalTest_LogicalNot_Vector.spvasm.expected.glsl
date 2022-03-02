#version 310 es
precision mediump float;

void main_1() {
  bvec2 x_1 = not(bvec2(true, false));
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
