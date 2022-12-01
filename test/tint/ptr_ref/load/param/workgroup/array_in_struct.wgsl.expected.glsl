#version 310 es

struct str {
  int arr[4];
};

shared str S;
int[4] func_S_arr() {
  return S.arr;
}

void tint_symbol(uint local_invocation_index) {
  {
    for(uint idx = local_invocation_index; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      S.arr[i] = 0;
    }
  }
  barrier();
  int r[4] = func_S_arr();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol(gl_LocalInvocationIndex);
  return;
}
