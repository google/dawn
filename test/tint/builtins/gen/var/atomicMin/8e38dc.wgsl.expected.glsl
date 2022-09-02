#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer SB_RW_ssbo {
  int arg_0;
} sb_rw;

void atomicMin_8e38dc() {
  int arg_1 = 1;
  int res = atomicMin(sb_rw.arg_0, arg_1);
}

void fragment_main() {
  atomicMin_8e38dc();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer SB_RW_ssbo {
  int arg_0;
} sb_rw;

void atomicMin_8e38dc() {
  int arg_1 = 1;
  int res = atomicMin(sb_rw.arg_0, arg_1);
}

void compute_main() {
  atomicMin_8e38dc();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
