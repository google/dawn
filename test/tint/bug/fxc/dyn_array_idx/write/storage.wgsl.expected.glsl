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

struct Result {
  int tint_symbol;
};

layout(binding = 2, std430) buffer result_block_ssbo {
  Result inner;
} result;

struct SSBO {
  int data[4];
};

layout(binding = 1, std430) buffer ssbo_block_ssbo {
  SSBO inner;
} ssbo;

void f() {
  ssbo.inner.data[ubo.inner.dynamic_idx] = 1;
  result.inner.tint_symbol = ssbo.inner.data[3];
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
