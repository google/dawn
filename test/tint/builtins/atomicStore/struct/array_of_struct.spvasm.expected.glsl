#version 310 es


struct S_atomic {
  int x;
  uint a;
  uint y;
};

uint local_invocation_index_1 = 0u;
shared S_atomic wg[10];
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  {
    while(true) {
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
      continue;
    }
  }
  barrier();
  atomicExchange(wg[4].a, 1u);
}
void compute_main_1() {
  uint x_53 = local_invocation_index_1;
  compute_main_inner(x_53);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint v = 0u;
    v = local_invocation_index_1_param;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 10u)) {
        break;
      }
      wg[v_1].x = 0;
      atomicExchange(wg[v_1].a, 0u);
      wg[v_1].y = 0u;
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
