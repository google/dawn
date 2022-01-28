#version 310 es
precision mediump float;

struct S {
  int a;
  float b;
};

shared S v;
void tint_symbol(uint local_invocation_index) {
  {
    S tint_symbol_1 = S(0, 0.0f);
    v = tint_symbol_1;
  }
  memoryBarrierShared();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
