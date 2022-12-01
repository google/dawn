#version 310 es

struct str {
  int i;
};

void func(inout str pointer) {
  str tint_symbol_1 = str(0);
  pointer = tint_symbol_1;
}

str P[4] = str[4](str(0), str(0), str(0), str(0));
void tint_symbol() {
  func(P[2]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
