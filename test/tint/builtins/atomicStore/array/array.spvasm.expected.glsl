#version 310 es

uint local_invocation_index_1 = 0u;
shared uint wg[4];
void compute_main_inner(uint local_invocation_index) {
  uint idx = 0u;
  idx = local_invocation_index;
  {
    for(; !(!((idx < 4u))); idx = (idx + 1u)) {
      atomicExchange(wg[idx], 0u);
    }
  }
  barrier();
  atomicExchange(wg[1], 1u);
  return;
}

void compute_main_1() {
  compute_main_inner(local_invocation_index_1);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  {
    for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 4u); idx_1 = (idx_1 + 1u)) {
      uint i = idx_1;
      atomicExchange(wg[i], 0u);
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
