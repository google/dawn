#version 310 es


struct S_atomic {
  int x;
  uint a[10];
  uint y;
};

uint local_invocation_index_1 = 0u;
shared S_atomic wg;
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  wg.x = 0;
  wg.y = 0u;
  idx = local_invocation_index_2;
  {
    while(true) {
      if (!((idx < 10u))) {
        break;
      }
      uint x_35 = idx;
      atomicExchange(wg.a[x_35], 0u);
      {
        idx = (idx + 1u);
      }
      continue;
    }
  }
  barrier();
  atomicExchange(wg.a[4], 1u);
}
void compute_main_1() {
  uint x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  if ((local_invocation_index_1_param == 0u)) {
    wg.x = 0;
    wg.y = 0u;
  }
  {
    uint v = 0u;
    v = local_invocation_index_1_param;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 10u)) {
        break;
      }
      atomicExchange(wg.a[v_1], 0u);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  barrier();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner_1(gl_LocalInvocationIndex);
}
