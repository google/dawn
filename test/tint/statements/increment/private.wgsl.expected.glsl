#version 310 es

int i = 0;
void v() {
  uint v_1 = uint(i);
  i = int((v_1 + uint(1)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
