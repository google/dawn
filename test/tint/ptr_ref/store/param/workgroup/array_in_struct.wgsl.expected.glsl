#version 310 es

struct str {
  int arr[4];
};

shared str S;
void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      S.arr[i] = 0;
    }
  }
  barrier();
}

void func_S_arr() {
  int tint_symbol_1[4] = int[4](0, 0, 0, 0);
  S.arr = tint_symbol_1;
}

void tint_symbol(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  func_S_arr();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
