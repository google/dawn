#version 310 es

int I = 0;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  I = 123;
  I = 123;
}
