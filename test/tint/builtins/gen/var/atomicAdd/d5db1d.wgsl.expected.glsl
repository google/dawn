#version 310 es

shared uint arg_0;
layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

void atomicAdd_d5db1d() {
  uint arg_1 = 1u;
  uint res = atomicAdd(arg_0, arg_1);
  prevent_dce.inner = res;
}

void compute_main(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  atomicAdd_d5db1d();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
