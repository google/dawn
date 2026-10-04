#version 310 es

shared float a[10];
shared float b[20];
void f_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 10u)) {
        break;
      }
      a[v] = 0.0f;
      {
        idx = (idx + 1u);
      }
    }
  }
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 20u)) {
        break;
      }
      b[v_1] = 0.0f;
      {
        idx = (idx + 1u);
      }
    }
  }
  barrier();
  float x = a[0];
  float y = b[0];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
