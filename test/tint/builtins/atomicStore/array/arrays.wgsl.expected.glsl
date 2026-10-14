#version 310 es

shared uint wg[3][2][1];
void compute_main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 6u)) {
        break;
      }
      atomicExchange(wg[(v / 2u)][(v % 2u)][0u], 0u);
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  atomicExchange(wg[2][1][0], 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main_inner(gl_LocalInvocationIndex);
}
