#version 310 es

layout(binding = 0, std430) buffer arr_block_ssbo {
  uint inner[];
} arr;

uint f2_arr_inner() {
  return uint(arr.inner.length());
}

uint f1_arr_inner() {
  return f2_arr_inner();
}

uint f0_arr_inner() {
  return f1_arr_inner();
}

void tint_symbol() {
  arr.inner[0] = f0_arr_inner();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
