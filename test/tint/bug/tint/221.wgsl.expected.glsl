#version 310 es

struct Buf {
  uint count;
  uint data[50];
};

layout(binding = 0, std430) buffer b_block_ssbo {
  Buf inner;
} b;

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % ((rhs == 0u) ? 1u : rhs));
}

void tint_symbol() {
  uint i = 0u;
  while (true) {
    if ((i >= b.inner.count)) {
      break;
    }
    uint p_save = i;
    if ((tint_mod(i, 2u) == 0u)) {
      {
        b.inner.data[p_save] = (b.inner.data[p_save] * 2u);
        i = (i + 1u);
      }
      continue;
    }
    b.inner.data[p_save] = 0u;
    {
      b.inner.data[p_save] = (b.inner.data[p_save] * 2u);
      i = (i + 1u);
    }
  }
  return;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
