#version 310 es
precision mediump float;

shared int v[3];

struct tint_symbol_2 {
  uint local_invocation_index;
};

void tint_symbol_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 3u); idx = (idx + 1u)) {
      uint i = idx;
      v[i] = 0;
    }
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


