#version 310 es

int s;
void f(int tint_symbol) {
  int b = tint_symbol;
  s = b;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f(1);
}
