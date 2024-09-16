#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_1_1_ssbo {
  uint tint_symbol;
} v;
shared uint arg_0;
uint atomicAdd_d5db1d() {
  uint res = atomicAdd(arg_0, 1u);
  return res;
}
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  v.tint_symbol = atomicAdd_d5db1d();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
