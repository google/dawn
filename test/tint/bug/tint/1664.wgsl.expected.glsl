#version 310 es

void f0() {
  int b = 1;
  int c = -2147483648;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f0();
  return;
}
