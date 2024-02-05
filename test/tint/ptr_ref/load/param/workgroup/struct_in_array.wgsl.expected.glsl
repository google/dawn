#version 310 es

struct str {
  int i;
};

shared str S[4];
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i_1 = idx;
      str tint_symbol_1 = str(0);
      S[i_1] = tint_symbol_1;
    }
  }
  barrier();
}

str func_S_X(uint pointer[1]) {
  return S[pointer[0]];
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  uint tint_symbol_2[1] = uint[1](2u);
  str r = func_S_X(tint_symbol_2);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
