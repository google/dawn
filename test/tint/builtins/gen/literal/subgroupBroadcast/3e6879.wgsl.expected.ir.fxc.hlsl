SKIP: FAILED

int2 subgroupBroadcast_3e6879() {
  int2 res = WaveReadLaneAt((1).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_3e6879();
}

