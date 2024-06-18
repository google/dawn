SKIP: FAILED

uint4 subgroupBroadcast_279027() {
  uint4 res = WaveReadLaneAt((1u).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_279027();
}

