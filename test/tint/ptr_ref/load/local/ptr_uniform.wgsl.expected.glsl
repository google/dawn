#version 310 es

struct S {
  int a;
};

layout(binding = 0) uniform S_1 {
  int a;
} v;

void tint_symbol() {
  int u = (v.a + 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
