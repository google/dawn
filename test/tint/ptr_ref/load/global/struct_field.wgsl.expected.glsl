#version 310 es

struct S {
  int i;
};

S V = S(0);
void tint_symbol() {
  int i = V.i;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
