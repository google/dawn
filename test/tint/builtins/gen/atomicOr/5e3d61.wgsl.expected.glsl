#version 310 es

shared uint arg_0;
void atomicOr_5e3d61() {
  uint res = atomicOr(arg_0, 1u);
}

void compute_main(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  atomicOr_5e3d61();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
