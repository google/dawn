#version 310 es

shared uint v[16];
void foo() {
  v[((min(0u, (4u - 1u)) * 8u) / 4u)] = 1065353216u;
}
void main_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 16u)) {
        break;
      }
      v[v_2] = 0u;
      {
        v_1 = (v_2 + 1u);
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
