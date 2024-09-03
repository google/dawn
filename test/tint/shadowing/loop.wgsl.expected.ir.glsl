#version 310 es

int tint_symbol[10];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  int i = 0;
  {
    while(true) {
      int x = tint_symbol[i];
      {
        int x = tint_symbol[x];
        i = (i + x);
        if ((i > 10)) { break; }
      }
      continue;
    }
  }
  tint_symbol[0] = i;
}
