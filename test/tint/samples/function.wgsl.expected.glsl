#version 310 es

void ep() {
}

layout(local_size_x = 2, local_size_y = 1, local_size_z = 1) in;
void main() {
  ep();
  return;
}
