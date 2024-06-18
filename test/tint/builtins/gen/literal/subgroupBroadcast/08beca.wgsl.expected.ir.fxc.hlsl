SKIP: FAILED

float subgroupBroadcast_08beca() {
  float res = WaveReadLaneAt(1.0f, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_08beca();
}

