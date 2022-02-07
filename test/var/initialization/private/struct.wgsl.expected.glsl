#version 310 es

struct S {
  int a;
  float b;
};

S v = S(0, 0.0f);
void tint_symbol() {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
