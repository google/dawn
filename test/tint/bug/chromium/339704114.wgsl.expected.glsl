#version 310 es

void b() {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  b();
  return;
}
