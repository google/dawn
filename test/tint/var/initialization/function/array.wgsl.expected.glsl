#version 310 es

void tint_symbol() {
  int v[3] = int[3](0, 0, 0);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
