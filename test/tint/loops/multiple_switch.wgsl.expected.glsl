#version 310 es

void tint_symbol() {
  int i = 0;
  {
    for(int i_1 = 0; (i_1 < 2); i_1 = (i_1 + 1)) {
      switch(i_1) {
        case 0: {
          continue;
          break;
        }
        default: {
          break;
        }
      }
      switch(i_1) {
        case 0: {
          continue;
          break;
        }
        default: {
          break;
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
