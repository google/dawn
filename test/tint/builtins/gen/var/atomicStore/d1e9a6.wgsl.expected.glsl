#version 310 es
precision highp float;

struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

void atomicStore_d1e9a6() {
  int arg_1 = 1;
  atomicExchange(sb_rw.inner.arg_0, arg_1);
}

void fragment_main() {
  atomicStore_d1e9a6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

void atomicStore_d1e9a6() {
  int arg_1 = 1;
  atomicExchange(sb_rw.inner.arg_0, arg_1);
}

void compute_main() {
  atomicStore_d1e9a6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
