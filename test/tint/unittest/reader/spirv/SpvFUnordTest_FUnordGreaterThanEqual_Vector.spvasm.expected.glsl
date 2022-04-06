#version 310 es
precision mediump float;

void main_1() {
  bvec2 x_1 = not(lessThan(vec2(50.0f, 60.0f), vec2(60.0f, 50.0f)));
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
