#version 310 es

void f() {
  int i = 0;
  int j = 0;
  mat2 m = mat2(vec2(1.0f, 2.0f), vec2(3.0f, 4.0f));
  float f_1 = m[i][j];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
