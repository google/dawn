SKIP: FAILED

int4 subgroupBroadcast_f637f9() {
  int4 res = WaveReadLaneAt((1).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_f637f9();
}

