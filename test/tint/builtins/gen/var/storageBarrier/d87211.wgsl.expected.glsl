#version 310 es

void storageBarrier_d87211() {
  { memoryBarrierBuffer(); barrier(); };
}

void compute_main() {
  storageBarrier_d87211();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
