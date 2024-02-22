#version 310 es

void tint_symbol() {
  bool tint_continue = false;
  {
    for(int i = 0; (i < 2); i = (i + 1)) {
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

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
