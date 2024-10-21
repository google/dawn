#version 310 es

layout(binding = 0, std430)
buffer tint_symbol_block_1_ssbo {
  int inner;
} v;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = v.inner;
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
  v.inner = i;
}
