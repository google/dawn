#version 310 es

shared uint arg_0;
void atomicAdd_d5db1d() {
  uint arg_1 = 1u;
  uint res = atomicAdd(arg_0, arg_1);
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
