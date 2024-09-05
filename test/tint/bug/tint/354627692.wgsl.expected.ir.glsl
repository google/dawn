#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_3_1_ssbo {
  int tint_symbol_2;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = v.tint_symbol_2;
  {
    while(true) {
      {
        {
          while(true) {
            if ((i > 5)) {
              i = (i * 2);
              break;
            } else {
              i = (i * 2);
              break;
            }
            /* unreachable */
          }
        }
        if ((i > 10)) { break; }
      }
      continue;
    }
  }
  v.tint_symbol_2 = i;
}
