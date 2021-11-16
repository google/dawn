#version 310 es
precision mediump float;

float v = 0.0f;

void x(inout float p) {
  p = 0.0f;
}

void g() {
  x(v);
}

void f() {
  g();
  return;
}
void main() {
  f();
}


