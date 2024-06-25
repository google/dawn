#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

struct SB_RW {
  int arg_0;
};

layout(binding = 1, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

int atomicMin_8e38dc() {
  int res = atomicMin(sb_rw.inner.arg_0, 1);
  return res;
}

void fragment_main() {
  prevent_dce.inner = atomicMin_8e38dc();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  int inner;
} prevent_dce;

struct SB_RW {
  int arg_0;
};

layout(binding = 1, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

int atomicMin_8e38dc() {
  int res = atomicMin(sb_rw.inner.arg_0, 1);
  return res;
}

void compute_main() {
  prevent_dce.inner = atomicMin_8e38dc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
