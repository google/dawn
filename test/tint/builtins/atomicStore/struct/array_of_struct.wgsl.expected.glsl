#version 310 es

struct S {
  int x;
  uint a;
  uint y;
};

shared S wg[10];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 10u); idx = (idx + 1u)) {
      uint i = idx;
      wg[i].x = 0;
      atomicExchange(wg[i].a, 0u);
      wg[i].y = 0u;
    }
  }
  barrier();
}

void compute_main(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicExchange(wg[4].a, 1u);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
