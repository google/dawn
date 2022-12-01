#version 310 es

struct str {
  ivec4 arr[4];
};

layout(binding = 0, std140) uniform S_block_ubo {
  str inner;
} S;

ivec4[4] func_S_arr() {
  return S.inner.arr;
}

void tint_symbol() {
  ivec4 r[4] = func_S_arr();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
