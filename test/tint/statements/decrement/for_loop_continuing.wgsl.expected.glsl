#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 0, std430) buffer i_block_ssbo {
  uint inner;
} i;

void tint_symbol() {
  {
    for(; (i.inner < 10u); i.inner = (i.inner - 1u)) {
    }
  }
}

