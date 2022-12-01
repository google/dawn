#version 310 es

struct str {
  int i;
};

int func(inout int pointer) {
  return pointer;
}

void tint_symbol() {
  str F = str(0);
  int r = func(F.i);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
