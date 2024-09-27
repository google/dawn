#version 310 es

uint tint_div(uint lhs, uint rhs) {
  return (lhs / mix(rhs, 1u, (rhs == 0u)));
}

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % mix(rhs, 1u, (rhs == 0u)));
}

shared int zero[2][3];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 6u); idx = (idx + 1u)) {
      uint i = tint_div(idx, 3u);
      uint i_1 = tint_mod(idx, 3u);
      zero[i][i_1] = 0;
    }
  }
  barrier();
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  int v[2][3] = zero;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
