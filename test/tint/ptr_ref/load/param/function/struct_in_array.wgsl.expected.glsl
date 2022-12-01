#version 310 es

struct str {
  int i;
};

str func(inout str pointer) {
  return pointer;
}

void tint_symbol() {
  str F[4] = str[4](str(0), str(0), str(0), str(0));
  str r = func(F[2]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
