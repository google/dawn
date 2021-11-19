#version 310 es
precision mediump float;

struct S {
  int a;
  float b;
};

shared S v;

struct tint_symbol_2 {
  uint local_invocation_index;
};

void tint_symbol_inner(uint local_invocation_index) {
  {
    S tint_symbol_3 = S(0, 0.0f);
    v = tint_symbol_3;
  }
  memoryBarrierShared();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol(tint_symbol_2 tint_symbol_1) {
  tint_symbol_inner(tint_symbol_1.local_invocation_index);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  tint_symbol(inputs);
}


