#version 310 es

void f() {
  {
    for(int i = 0; (i < 4); i = (i + 1)) {
      switch(i) {
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
  f();
  return;
}
