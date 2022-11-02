#version 310 es

struct UBO {
  int dynamic_idx;
  uint pad;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std140) uniform ubo_block_ubo {
  UBO inner;
} ubo;

struct S {
  int data[64];
};

struct Result {
  int tint_symbol;
};

layout(binding = 1, std430) buffer result_block_ssbo {
  Result inner;
} result;

void x(inout S p) {
  p.data[ubo.inner.dynamic_idx] = 1;
}

void f() {
  S s = S(int[64](0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
  x(s);
  result.inner.tint_symbol = s.data[3];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
