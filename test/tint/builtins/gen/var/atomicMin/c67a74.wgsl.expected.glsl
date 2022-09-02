#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer SB_RW_ssbo {
  uint arg_0;
} sb_rw;

void atomicMin_c67a74() {
  uint arg_1 = 1u;
  uint res = atomicMin(sb_rw.arg_0, arg_1);
}

void fragment_main() {
  atomicMin_c67a74();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer SB_RW_ssbo {
  uint arg_0;
} sb_rw;

void atomicMin_c67a74() {
  uint arg_1 = 1u;
  uint res = atomicMin(sb_rw.arg_0, arg_1);
}

void compute_main() {
  atomicMin_c67a74();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
