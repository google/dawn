#version 310 es

struct S {
  bool e;
};

void tint_symbol() {
  bool b = false;
  S v = S(bool(uint(true) & uint(b)));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
