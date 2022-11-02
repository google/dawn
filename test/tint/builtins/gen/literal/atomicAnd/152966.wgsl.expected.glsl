#version 310 es
precision mediump float;

struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

void atomicAnd_152966() {
  int res = atomicAnd(sb_rw.inner.arg_0, 1);
}

void fragment_main() {
  atomicAnd_152966();
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

void atomicAnd_152966() {
  int res = atomicAnd(sb_rw.inner.arg_0, 1);
}

void compute_main() {
  atomicAnd_152966();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
