#version 310 es

struct Inner {
  float f;
  uint pad;
  uint pad_1;
  uint pad_2;
};

struct S {
  Inner inner;
};

layout(binding = 0, std140) uniform u_block_ubo {
  S inner;
} u;

void tint_symbol() {
  S x = u.inner;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
