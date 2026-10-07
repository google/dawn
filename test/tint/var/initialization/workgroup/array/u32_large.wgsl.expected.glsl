#version 310 es

shared int zero[23];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 23u)) {
        break;
      }
      zero[v_1] = 0;
      {
        idx = (idx + 13u);
      }
    }
  }
  barrier();
  int v[23] = zero;
}
layout(local_size_x = 13, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
