#version 310 es

shared uint v[16];
void foo() {
  uint v_1 = ((min(uint(0), (4u - 1u)) * 8u) / 4u);
  v[v_1] = 1065353216u;
}
void main_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 16u)) {
        break;
      }
      v[v_3] = 0u;
      {
        v_2 = (v_3 + 1u);
      }
    }
  }
  barrier();
  foo();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
