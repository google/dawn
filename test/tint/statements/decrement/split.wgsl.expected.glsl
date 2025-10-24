#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int b = 2;
  int v = b;
  int v_1 = int((~(uint(b)) + 1u));
  uint v_2 = uint(v);
  int c = int((v_2 - uint(v_1)));
}
