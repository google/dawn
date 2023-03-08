#version 310 es

int tint_mod(int lhs, int rhs) {
  int rhs_or_one = (bool(uint((rhs == 0)) | uint(bool(uint((lhs == (-2147483647 - 1))) & uint((rhs == -1))))) ? 1 : rhs);
  if (((uint((lhs | rhs_or_one)) & 2147483648u) != 0u)) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

void f() {
  int a = 1;
  int b = 0;
  int r = tint_mod(a, b);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
