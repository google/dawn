#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;

void atomicLoad_fe6cc3() {
  uint res = atomicOr(sb_rw.arg_0, 0u);
}

void fragment_main() {
  atomicLoad_fe6cc3();
  return;
}
void main() {
  fragment_main();
}


#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;

void atomicLoad_fe6cc3() {
  uint res = atomicOr(sb_rw.arg_0, 0u);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicLoad_fe6cc3();
  return;
}
void main() {
  compute_main();
}


