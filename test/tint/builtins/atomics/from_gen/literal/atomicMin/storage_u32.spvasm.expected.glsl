#version 310 es
precision highp float;

struct SB_RW_atomic {
  uint arg_0;
};

struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW_atomic inner;
} sb_rw;

void atomicMin_c67a74() {
  uint res = 0u;
  uint x_9 = atomicMin(sb_rw.inner.arg_0, 1u);
  res = x_9;
  return;
}

void fragment_main_1() {
  atomicMin_c67a74();
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

struct SB_RW_atomic {
  uint arg_0;
};

struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW_atomic inner;
} sb_rw;

void atomicMin_c67a74() {
  uint res = 0u;
  uint x_9 = atomicMin(sb_rw.inner.arg_0, 1u);
  res = x_9;
  return;
}

void compute_main_1() {
  atomicMin_c67a74();
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
