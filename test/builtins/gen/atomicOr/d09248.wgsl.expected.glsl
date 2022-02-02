#version 310 es
precision mediump float;

shared int arg_0;
void atomicOr_d09248() {
  int res = atomicOr(arg_0, 1);
}

void compute_main(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0);
  }
  memoryBarrierShared();
  atomicOr_d09248();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
