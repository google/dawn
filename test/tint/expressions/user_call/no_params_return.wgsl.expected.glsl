#version 310 es

int c() {
  int a = 1;
  uint v = uint(a);
  a = int((v + uint(2)));
  return a;
}
void b() {
  int b_1 = c();
  int v_1 = c();
  uint v_2 = uint(b_1);
  b_1 = int((v_2 + uint(v_1)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
