SKIP: FAILED

uint subgroupBroadcast_c36fe1() {
  uint res = WaveReadLaneAt(1u, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_c36fe1();
}

