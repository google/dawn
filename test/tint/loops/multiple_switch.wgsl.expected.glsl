#version 310 es

void tint_symbol() {
  int i = 0;
  bool tint_continue = false;
  {
    for(int i_1 = 0; (i_1 < 2); i_1 = (i_1 + 1)) {
      tint_continue = false;
      switch(i_1) {
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
      switch(i_1) {
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

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
