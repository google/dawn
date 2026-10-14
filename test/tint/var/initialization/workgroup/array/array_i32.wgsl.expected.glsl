#version 310 es

shared int zero[2][3];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 6u)) {
        break;
      }
      zero[(v_1 / 3u)][(v_1 % 3u)] = 0;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  int v[2][3] = zero;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
