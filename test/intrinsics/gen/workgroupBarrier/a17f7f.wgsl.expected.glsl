#version 310 es
precision mediump float;

void workgroupBarrier_a17f7f() {
  memoryBarrierShared();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  workgroupBarrier_a17f7f();
  return;
}
void main() {
  compute_main();
}


