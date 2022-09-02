#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer SB_RW_ssbo {
  uint arg_0;
} sb_rw;

void atomicLoad_fe6cc3() {
  uint res = atomicOr(sb_rw.arg_0, 0u);
}

void fragment_main() {
  atomicLoad_fe6cc3();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer SB_RW_ssbo {
  uint arg_0;
} sb_rw;

void atomicLoad_fe6cc3() {
  uint res = atomicOr(sb_rw.arg_0, 0u);
}

void compute_main() {
  atomicLoad_fe6cc3();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
