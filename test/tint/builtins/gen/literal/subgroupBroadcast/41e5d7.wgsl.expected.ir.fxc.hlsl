SKIP: FAILED

vector<float16_t, 3> subgroupBroadcast_41e5d7() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_41e5d7();
}

