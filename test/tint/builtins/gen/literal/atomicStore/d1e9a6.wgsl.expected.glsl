#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer SB_RW_ssbo {
  int arg_0;
} sb_rw;

void atomicStore_d1e9a6() {
  atomicExchange(sb_rw.arg_0, 1);
}

void fragment_main() {
  atomicStore_d1e9a6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer SB_RW_ssbo {
  int arg_0;
} sb_rw;

void atomicStore_d1e9a6() {
  atomicExchange(sb_rw.arg_0, 1);
}

void compute_main() {
  atomicStore_d1e9a6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
