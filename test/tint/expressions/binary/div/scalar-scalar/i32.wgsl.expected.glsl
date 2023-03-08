#version 310 es

int tint_div(int lhs, int rhs) {
  return (lhs / (bool(uint((rhs == 0)) | uint(bool(uint((lhs == (-2147483647 - 1))) & uint((rhs == -1))))) ? 1 : rhs));
}

void f() {
  int a = 1;
  int b = 2;
  int r = tint_div(a, b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
