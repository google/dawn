#version 310 es

shared float a[2];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 2u)) {
        break;
      }
      a[v] = 0.0f;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
