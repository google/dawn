#version 310 es

void tint_symbol() {
}

layout(local_size_x = 3, local_size_y = 2, local_size_z = 3) in;
void main() {
  tint_symbol();
  return;
}
