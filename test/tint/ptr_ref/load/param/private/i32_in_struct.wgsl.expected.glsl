#version 310 es

struct str {
  int i;
};

int func(inout int pointer) {
  return pointer;
}

str P = str(0);
void tint_symbol() {
  int r = func(P.i);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
