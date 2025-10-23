#version 310 es

shared float a[10];
shared float b[20];
void f_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 10u)) {
        break;
      }
      a[v_1] = 0.0f;
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 20u)) {
        break;
      }
      b[v_3] = 0.0f;
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  barrier();
  float x = a[0u];
  float y = b[0u];
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f_inner(gl_LocalInvocationIndex);
}
