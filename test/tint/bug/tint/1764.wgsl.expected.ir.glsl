#version 310 es

shared int W[246];
void tint_symbol_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 246u)) {
        break;
      }
      W[v_1] = 0;
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  barrier();
  W[0] = 42;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
