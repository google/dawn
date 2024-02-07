#version 310 es
precision highp float;
precision highp int;

struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

void atomicStore_cdc29e() {
  atomicExchange(sb_rw.inner.arg_0, 1u);
}

void fragment_main() {
  atomicStore_cdc29e();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

void atomicStore_cdc29e() {
  atomicExchange(sb_rw.inner.arg_0, 1u);
}

void compute_main() {
  atomicStore_cdc29e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
