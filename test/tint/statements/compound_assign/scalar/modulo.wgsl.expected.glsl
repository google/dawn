#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  int a;
};

layout(binding = 0, std430) buffer v_block_ssbo {
  S inner;
} v;

int tint_mod(int lhs, int rhs) {
  return (lhs % (bool(uint((rhs == 0)) | uint(bool(uint((lhs == -2147483648)) & uint((rhs == -1))))) ? 1 : rhs));
}

void foo() {
  int tint_symbol = tint_mod(v.inner.a, 2);
  v.inner.a = tint_symbol;
}

