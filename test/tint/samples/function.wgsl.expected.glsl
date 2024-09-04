#version 310 es

float tint_symbol() {
  return 0.40000000596046447754f;
}

void ep() {
  float a = tint_symbol();
}

layout(local_size_x = 2, local_size_y = 1, local_size_z = 1) in;
void main() {
  ep();
  return;
}
