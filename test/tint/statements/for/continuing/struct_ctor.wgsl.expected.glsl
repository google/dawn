#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  int i;
};

void f() {
  {
    int i = 0;
    while (true) {
      if (true) {
        break;
      }
      {
      }
      {
        S tint_symbol = S(1);
        i = (i + tint_symbol.i);
      }
    }
  }
}

