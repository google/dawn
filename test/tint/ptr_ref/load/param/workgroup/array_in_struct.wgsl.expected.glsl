#version 310 es


struct str {
  int arr[4];
};

shared str S;
int[4] func() {
  return S.arr;
}
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 4u)) {
        break;
      }
      S.arr[v] = 0;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  int r[4] = func();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
