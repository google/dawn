#version 310 es
precision highp float;

struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void atomicMax_92aa72() {
  int res = atomicMax(sb_rw.inner.arg_0, 1);
  prevent_dce.inner = res;
}

void fragment_main() {
  atomicMax_92aa72();
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

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

void atomicMax_92aa72() {
  int res = atomicMax(sb_rw.inner.arg_0, 1);
  prevent_dce.inner = res;
}

void compute_main() {
  atomicMax_92aa72();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
