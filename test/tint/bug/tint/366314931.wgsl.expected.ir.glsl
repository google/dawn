#version 310 es


struct S {
  uvec3 v;
  uint u;
};

shared S wgvar;
layout(binding = 0, std430)
buffer tint_symbol_block_1_ssbo {
  S inner;
} v_1;
void tint_symbol_1_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    wgvar.v = uvec3(0u);
    atomicExchange(wgvar.u, 0u);
  }
  barrier();
  uint x = atomicOr(wgvar.u, 0u);
  atomicExchange(v_1.inner.u, x);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1_inner(gl_LocalInvocationIndex);
}
