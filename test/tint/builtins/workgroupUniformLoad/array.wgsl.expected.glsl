#version 310 es

shared int v[4];
int[4] foo() {
  barrier();
  int v_1[4] = v;
  barrier();
  return v_1;
}
void main_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      v[v_3] = 0;
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  barrier();
  foo();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
