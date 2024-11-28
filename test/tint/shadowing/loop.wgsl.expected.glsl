#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_block_1_ssbo {
  int inner[10];
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = 0;
  {
    while(true) {
      uint v_1 = min(uint(i), 9u);
      int x = v.inner[v_1];
      {
        uint v_2 = min(uint(x), 9u);
        int x_1 = v.inner[v_2];
        i = (i + x_1);
        if ((i > 10)) { break; }
      }
      continue;
    }
  }
  v.inner[0u] = i;
}
