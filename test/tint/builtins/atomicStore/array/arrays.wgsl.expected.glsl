#version 310 es

shared uint wg[3][2][1];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 6u); idx = (idx + 1u)) {
      uint i = (idx / 2u);
      uint i_1 = (idx % 2u);
      uint i_2 = (idx % 1u);
      atomicExchange(wg[i][i_1][i_2], 0u);
    }
  }
  barrier();
}

void compute_main(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  atomicExchange(wg[2][1][0], 1u);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
