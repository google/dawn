<dawn>/test/tint/bug/tint/1474-b.wgsl:7:5 warning: code is unreachable
    let non_uniform_cond = non_uniform_value == 0;
    ^^^^^^^^^^^^^^^^^^^^

#version 310 es

layout(binding = 0, std430) buffer non_uniform_value_block_ssbo {
  int inner;
} non_uniform_value;

void tint_symbol() {
  return;
  bool non_uniform_cond = (non_uniform_value.inner == 0);
  if (non_uniform_cond) {
    barrier();
  }
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
