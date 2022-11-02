#version 310 es

struct UBO {
  ivec4 data[4];
  int dynamic_idx;
  uint pad;
  uint pad_1;
  uint pad_2;
};

layout(binding = 0, std140) uniform ubo_block_ubo {
  UBO inner;
} ubo;

struct Result {
  int tint_symbol;
};

layout(binding = 2, std430) buffer result_block_ssbo {
  Result inner;
} result;

void f() {
  result.inner.tint_symbol = ubo.inner.data[ubo.inner.dynamic_idx].x;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
