#version 310 es

int tint_symbol;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = tint_symbol;
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
  tint_symbol = i;
}
