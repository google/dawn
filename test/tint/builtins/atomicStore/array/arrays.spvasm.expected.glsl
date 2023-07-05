#version 310 es

uint local_invocation_index_1 = 0u;
shared uint wg[3][2][1];
uint tint_div(uint lhs, uint rhs) {
  return (lhs / ((rhs == 0u) ? 1u : rhs));
}

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % ((rhs == 0u) ? 1u : rhs));
}

void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  while (true) {
    if (!((idx < 6u))) {
      break;
    }
    uint x_31 = idx;
    uint x_33 = idx;
    uint x_35 = idx;
    atomicExchange(wg[tint_div(x_31, 2u)][tint_mod(x_33, 2u)][tint_mod(x_35, 1u)], 0u);
    {
      idx = (idx + 1u);
    }
  }
  barrier();
  atomicExchange(wg[2][1][0], 1u);
  return;
}

void compute_main_1() {
  uint x_57 = local_invocation_index_1;
  compute_main_inner(x_57);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  {
    for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 6u); idx_1 = (idx_1 + 1u)) {
      uint i = (idx_1 / 2u);
      uint i_1 = (idx_1 % 2u);
      uint i_2 = (idx_1 % 1u);
      atomicExchange(wg[i][i_1][i_2], 0u);
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
