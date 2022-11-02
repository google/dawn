#version 310 es
precision mediump float;

struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

void atomicXor_c1b78c() {
  int arg_1 = 1;
  int res = atomicXor(sb_rw.inner.arg_0, arg_1);
}

void fragment_main() {
  atomicXor_c1b78c();
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

void atomicXor_c1b78c() {
  int arg_1 = 1;
  int res = atomicXor(sb_rw.inner.arg_0, arg_1);
}

void compute_main() {
  atomicXor_c1b78c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
