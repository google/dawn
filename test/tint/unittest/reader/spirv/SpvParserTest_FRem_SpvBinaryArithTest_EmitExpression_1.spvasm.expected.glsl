#version 310 es
precision mediump float;

vec2 tint_float_modulo(vec2 lhs, vec2 rhs) {
  return (lhs - rhs * trunc(lhs / rhs));
}


void main_1() {
  vec2 x_1 = tint_float_modulo(vec2(50.0f, 60.0f), vec2(60.0f, 50.0f));
  return;
}

void tint_symbol() {
  main_1();
}

void main() {
  tint_symbol();
  return;
}
