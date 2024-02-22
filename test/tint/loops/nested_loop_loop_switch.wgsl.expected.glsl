#version 310 es

void tint_symbol() {
  {
    for(int i = 0; (i < 2); i = (i + 2)) {
      bool tint_continue = false;
      {
        for(int j = 0; (j < 2); j = (j + 2)) {
          tint_continue = false;
          switch(i) {
            case 0: {
              tint_continue = true;
              break;
            }
            default: {
              break;
            }
          }
          if (tint_continue) {
            continue;
          }
        }
      }
    }
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
