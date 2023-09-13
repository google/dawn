#version 310 es

void tint_symbol() {
  {
    for(int i = 0; (i < 2); i = (i + 1)) {
      switch(i) {
        case 0: {
          continue;
          break;
        }
        case 1: {
          continue;
          break;
        }
        case 2: {
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
