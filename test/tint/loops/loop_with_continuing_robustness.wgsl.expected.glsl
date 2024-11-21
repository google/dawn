#version 310 es

int f() {
  int i = 0;
  {
    while(true) {
      if ((i > 4)) {
        return i;
      }
      {
        i = (i + 1);
      }
      continue;
    }
  }
  /* unreachable */
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
