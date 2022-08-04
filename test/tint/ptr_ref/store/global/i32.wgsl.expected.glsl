#version 310 es

int I = 0;
void tint_symbol() {
  I = 123;
  I = 123;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
