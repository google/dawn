#version 310 es

struct Buffer {
  uint data;
};

Buffer tint_symbol;
void tint_symbol_1() {
  tint_symbol.data = (tint_symbol.data + 1u);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
