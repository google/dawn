#version 310 es

void textureBarrier_3d0f7e() {
  { memoryBarrierImage(); barrier(); };
}

void compute_main() {
  textureBarrier_3d0f7e();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
