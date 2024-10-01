#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_2_1_ssbo {
  int tint_symbol_1[10];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = 0;
  {
    while(true) {
      int v_1 = i;
      int x = v.tint_symbol_1[v_1];
      {
        int v_2 = x;
        int x = v.tint_symbol_1[v_2];
        i = (i + x);
        if ((i > 10)) { break; }
      }
      continue;
    }
  }
  v.tint_symbol_1[0] = i;
}
