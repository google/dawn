#version 310 es

struct S_atomic {
  int x;
  uint a[10];
  uint y;
};

struct S {
  int x;
  uint a[10];
  uint y;
};

uint local_invocation_index_1 = 0u;
shared S_atomic wg;
void compute_main_inner(uint local_invocation_index) {
  uint idx = 0u;
  wg.x = 0;
  wg.y = 0u;
  idx = local_invocation_index;
  {
    for(; !(!((idx < 10u))); idx = (idx + 1u)) {
      atomicExchange(wg.a[idx], 0u);
    }
  }
  barrier();
  atomicExchange(wg.a[4], 1u);
  return;
}

void compute_main_1() {
  compute_main_inner(local_invocation_index_1);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  {
    wg.x = 0;
    wg.y = 0u;
  }
  {
    for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 10u); idx_1 = (idx_1 + 1u)) {
      uint i = idx_1;
      atomicExchange(wg.a[i], 0u);
    }
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
