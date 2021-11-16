#version 310 es
precision mediump float;

int f(int a, int b, int c) {
  return ((a * b) + c);
}

void phony_sink(int p0, int p1, int p2) {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  phony_sink(f(1, 2, 3), f(4, 5, 6), f(7, f(8, 9, 10), 11));
  return;
}
void main() {
  tint_symbol();
}


