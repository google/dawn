#version 310 es

int i = 123;
void tint_symbol() {
  int u = (i + 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
