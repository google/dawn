#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicXor_c1b78c() {
  int res = atomicXor(sb_rw.arg_0, 1);
}

void fragment_main() {
  atomicXor_c1b78c();
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

void atomicXor_c1b78c() {
  int res = atomicXor(sb_rw.arg_0, 1);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicXor_c1b78c();
  return;
}
void main() {
  compute_main();
}


