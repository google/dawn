#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicStore_d1e9a6() {
  atomicExchange(sb_rw.arg_0, 1);
}

void fragment_main() {
  atomicStore_d1e9a6();
  return;
}
void main() {
  fragment_main();
}


#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicStore_d1e9a6() {
  atomicExchange(sb_rw.arg_0, 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicStore_d1e9a6();
  return;
}
void main() {
  compute_main();
}


