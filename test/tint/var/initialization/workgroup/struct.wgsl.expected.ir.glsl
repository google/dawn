#version 310 es


struct S {
  int a;
  float b;
};

shared S v;
void tint_symbol_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v = S(0, 0.0f);
  }
  barrier();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
