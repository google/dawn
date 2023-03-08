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
  int rhs_or_one = (bool(uint((rhs == 0)) | uint(bool(uint((lhs == (-2147483647 - 1))) & uint((rhs == -1))))) ? 1 : rhs);
  if (((uint((lhs | rhs_or_one)) & 2147483648u) != 0u)) {
    return (lhs - ((lhs / rhs_or_one) * rhs_or_one));
  } else {
    return (lhs % rhs_or_one);
  }
}

void foo() {
  v.inner.a = tint_mod(v.inner.a, 2);
}

