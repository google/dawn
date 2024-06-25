#version 310 es
precision highp float;
precision highp int;

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

struct SB_RW {
  uint arg_0;
};

layout(binding = 1, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

uint atomicSub_15bfc9() {
  uint arg_1 = 1u;
  uint res = atomicAdd(sb_rw.inner.arg_0, -(arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.inner = atomicSub_15bfc9();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer prevent_dce_block_ssbo {
  uint inner;
} prevent_dce;

struct SB_RW {
  uint arg_0;
};

layout(binding = 1, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

uint atomicSub_15bfc9() {
  uint arg_1 = 1u;
  uint res = atomicAdd(sb_rw.inner.arg_0, -(arg_1));
  return res;
}

void compute_main() {
  prevent_dce.inner = atomicSub_15bfc9();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
