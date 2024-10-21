#version 310 es

uint local_invocation_index_1 = 0u;
shared uint wg[3][2][1];
uint tint_mod_u32(uint lhs, uint rhs) {
  uint v = mix(rhs, 1u, (rhs == 0u));
  return (lhs - ((lhs / v) * v));
}
uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}
void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  {
    while(true) {
      if (!((idx < 6u))) {
        break;
      }
      uint x_31 = idx;
      uint x_33 = idx;
      uint x_35 = idx;
      uint v_1 = tint_div_u32(x_31, 2u);
      uint v_2 = tint_mod_u32(x_33, 2u);
      uint v_3 = tint_mod_u32(x_35, 1u);
      atomicExchange(wg[v_1][v_2][v_3], 0u);
      {
        idx = (idx + 1u);
      }
      continue;
    }
  }
  barrier();
  atomicExchange(wg[2][1][0], 1u);
}
void compute_main_1() {
  uint x_57 = local_invocation_index_1;
  compute_main_inner(x_57);
}
void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint v_4 = 0u;
    v_4 = local_invocation_index_1_param;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 6u)) {
        break;
      }
      atomicExchange(wg[(v_5 / 2u)][(v_5 % 2u)][0u], 0u);
      {
        v_4 = (v_5 + 1u);
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
