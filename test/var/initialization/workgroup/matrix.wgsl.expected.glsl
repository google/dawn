#version 310 es
precision mediump float;

shared mat2x3 v;

struct tint_symbol_2 {
  uint local_invocation_index;
};

void tint_symbol_inner(uint local_invocation_index) {
  {
    v = mat2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
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


