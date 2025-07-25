#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int a = 1;
  int b = 2;
  uint v = uint(a);
  int r = int((v * uint(b)));
}
