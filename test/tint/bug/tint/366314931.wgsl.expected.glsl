#version 310 es

struct S {
  uvec3 v;
  uint u;
};

shared S wgvar;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    wgvar.v = uvec3(0u);
    atomicExchange(wgvar.u, 0u);
  }
  barrier();
}

layout(binding = 0, std430) buffer tint_symbol_block_ssbo {
  S inner;
} tint_symbol;

void tint_symbol_1(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint x = atomicOr(wgvar.u, 0u);
  atomicExchange(tint_symbol.inner.u, x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1(gl_LocalInvocationIndex);
  return;
}
