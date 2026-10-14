#version 310 es

shared int W[246];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 246u)) {
        break;
      }
      W[v] = 0;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  W[0] = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
