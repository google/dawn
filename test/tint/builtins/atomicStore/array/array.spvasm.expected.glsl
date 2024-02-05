#version 310 es

shared uint wg[4];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx_1 = local_idx; (idx_1 < 4u); idx_1 = (idx_1 + 1u)) {
      uint i = idx_1;
      atomicExchange(wg[i], 0u);
    }
  }
  barrier();
}

uint local_invocation_index_1 = 0u;
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  while (true) {
    if (!((idx < 4u))) {
      break;
    }
    uint x_26 = idx;
    atomicExchange(wg[x_26], 0u);
    {
      idx = (idx + 1u);
    }
  }
  barrier();
  atomicExchange(wg[1], 1u);
  return;
}

void compute_main_1() {
  uint x_47 = local_invocation_index_1;
  compute_main_inner(x_47);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  tint_zero_workgroup_memory(local_invocation_index_1_param);
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
