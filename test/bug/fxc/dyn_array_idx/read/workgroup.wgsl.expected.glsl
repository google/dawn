#version 310 es
precision mediump float;


layout (binding = 0) uniform UBO_1 {
  int dynamic_idx;
} ubo;

struct S {
  int data[64];
};

layout (binding = 1) buffer Result_1 {
  int tint_symbol;
} result;
shared S s;

struct tint_symbol_2 {
  uint local_invocation_index;
};

void f_inner(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 64u); idx = (idx + 1u)) {
      uint i = idx;
      s.data[i] = 0;
    }
  }
  memoryBarrierShared();
  result.tint_symbol = s.data[ubo.dynamic_idx];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void f(tint_symbol_2 tint_symbol_1) {
  f_inner(tint_symbol_1.local_invocation_index);
  return;
}
void main() {
  tint_symbol_2 inputs;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  f(inputs);
}


