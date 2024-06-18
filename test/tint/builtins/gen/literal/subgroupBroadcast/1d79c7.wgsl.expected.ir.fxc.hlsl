SKIP: FAILED

int subgroupBroadcast_1d79c7() {
  int res = WaveReadLaneAt(1, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_1d79c7();
}

