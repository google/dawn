#version 310 es

struct tint_symbol {
  int tint_symbol_1;
};

int s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol c = tint_symbol(0);
  int d = c.tint_symbol_1;
  s = (c.tint_symbol_1 + d);
}
