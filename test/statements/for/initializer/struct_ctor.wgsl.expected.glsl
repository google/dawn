#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}

void main() {
  unused_entry_point();
}

struct S {
  int i;
};

void f() {
  S tint_symbol = S(1);
  {
    for(int i = tint_symbol.i; false; ) {
    }
  }
}

