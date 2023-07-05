#version 310 es

struct S_atomic {
  int x;
  uint a;
  uint y;
};

struct S {
  int x;
  uint a;
  uint y;
};

uint local_invocation_index_1 = 0u;
shared S_atomic wg[10];
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  while (true) {
    if (!((idx < 10u))) {
      break;
    }
    uint x_28 = idx;
    wg[x_28].x = 0;
    atomicExchange(wg[x_28].a, 0u);
    wg[x_28].y = 0u;
    {
      idx = (idx + 1u);
    }
  }
  barrier();
  atomicExchange(wg[4].a, 1u);
  return;
}

void compute_main_1() {
  uint x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  {
    for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 10u); idx_1 = (idx_1 + 1u)) {
      uint i = idx_1;
      wg[i].x = 0;
      atomicExchange(wg[i].a, 0u);
      wg[i].y = 0u;
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
