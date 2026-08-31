#version 310 es

void c(int z) {
  int a = int((1u + uint(z)));
  a = int((uint(a) + 2u));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  c(2);
  c(3);
}
