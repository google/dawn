#version 310 es
precision mediump float;

struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer SB_RW_atomic_ssbo {
  uint arg_0;
} sb_rw;

void atomicStore_cdc29e() {
  atomicExchange(sb_rw.arg_0, 1u);
  return;
}

void fragment_main_1() {
  atomicStore_cdc29e();
  return;
}

void fragment_main() {
  fragment_main_1();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer SB_RW_atomic_ssbo {
  uint arg_0;
} sb_rw;

void atomicStore_cdc29e() {
  atomicExchange(sb_rw.arg_0, 1u);
  return;
}

void compute_main_1() {
  atomicStore_cdc29e();
  return;
}

void compute_main() {
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
