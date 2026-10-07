#version 310 es

shared uint wg[4];
void compute_main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 4u)) {
        break;
      }
      atomicExchange(wg[v], 0u);
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  atomicExchange(wg[1], 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
