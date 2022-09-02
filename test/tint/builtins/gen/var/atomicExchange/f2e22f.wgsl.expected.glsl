#version 310 es
precision mediump float;

layout(binding = 0, std430) buffer SB_RW_ssbo {
  int arg_0;
} sb_rw;

void atomicExchange_f2e22f() {
  int arg_1 = 1;
  int res = atomicExchange(sb_rw.arg_0, arg_1);
}

void fragment_main() {
  atomicExchange_f2e22f();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

layout(binding = 0, std430) buffer SB_RW_ssbo {
  int arg_0;
} sb_rw;

void atomicExchange_f2e22f() {
  int arg_1 = 1;
  int res = atomicExchange(sb_rw.arg_0, arg_1);
}

void compute_main() {
  atomicExchange_f2e22f();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
