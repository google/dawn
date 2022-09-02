#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer SB_RW_ssbo {
  uint arg_0;
} sb_rw;

void atomicStore_cdc29e() {
  uint arg_1 = 1u;
  atomicExchange(sb_rw.arg_0, arg_1);
}

void fragment_main() {
  atomicStore_cdc29e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer SB_RW_ssbo {
  uint arg_0;
} sb_rw;

void atomicStore_cdc29e() {
  uint arg_1 = 1u;
  atomicExchange(sb_rw.arg_0, arg_1);
}

void compute_main() {
  atomicStore_cdc29e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
