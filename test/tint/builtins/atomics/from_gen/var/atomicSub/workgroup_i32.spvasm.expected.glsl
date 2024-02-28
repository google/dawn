#version 310 es

shared int arg_0;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    atomicExchange(arg_0, 0);
  }
  barrier();
}

uint local_invocation_index_1 = 0u;
void atomicSub_77883a() {
  int arg_1 = 0;
  int res = 0;
  arg_1 = 1;
  int x_19 = arg_1;
  int x_15 = atomicAdd(arg_0, -(x_19));
  res = x_15;
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0);
  barrier();
  atomicSub_77883a();
  return;
}

void compute_main_1() {
  uint x_33 = local_invocation_index_1;
  compute_main_inner(x_33);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  tint_zero_workgroup_memory(local_invocation_index_1_param);
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
