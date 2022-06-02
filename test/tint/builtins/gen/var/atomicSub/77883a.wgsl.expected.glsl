#version 310 es

shared int arg_0;
void atomicSub_77883a() {
  int arg_1 = 1;
  int res = atomicAdd(arg_0, arg_1);
}

void compute_main(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0);
  }
  barrier();
  atomicSub_77883a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
