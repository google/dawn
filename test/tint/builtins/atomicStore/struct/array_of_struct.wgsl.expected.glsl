#version 310 es


struct S {
  int x;
  uint a;
  uint y;
};

shared S wg[10];
void compute_main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 10u)) {
        break;
      }
      wg[v].x = 0;
      atomicExchange(wg[v].a, 0u);
      wg[v].y = 0u;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  atomicExchange(wg[4].a, 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
