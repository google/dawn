#version 310 es

struct S {
  int x;
  uint a[10];
  uint y;
};

shared S wg;
void compute_main(uint local_invocation_index) {
  {
    wg.x = 0;
    wg.y = 0u;
  }
  {
    for(uint idx = local_invocation_index; (idx < 10u); idx = (idx + 1u)) {
      uint i = idx;
      atomicExchange(wg.a[i], 0u);
    }
  }
  barrier();
  atomicExchange(wg.a[4], 1u);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
