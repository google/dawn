#version 310 es

struct str {
  int arr[4];
};

void func(inout int pointer[4]) {
  int tint_symbol_1[4] = int[4](0, 0, 0, 0);
  pointer = tint_symbol_1;
}

str P = str(int[4](0, 0, 0, 0));
void tint_symbol() {
  func(P.arr);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
