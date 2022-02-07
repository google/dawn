#version 310 es

int f(int a, int b, int c) {
  return ((a * b) + c);
}

void tint_symbol() {
  f(1, 2, 3);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
