#version 310 es

struct S {
  int a;
};

layout(binding = 0, std140) uniform v_block_ubo {
  S inner;
} v;

void tint_symbol() {
  int u = (v.inner.a + 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
