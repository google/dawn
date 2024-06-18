SKIP: FAILED

int3 subgroupBroadcast_e275c8() {
  int3 res = WaveReadLaneAt((1).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_e275c8();
}

