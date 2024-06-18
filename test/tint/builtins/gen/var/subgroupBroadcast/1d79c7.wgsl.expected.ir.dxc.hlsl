SKIP: FAILED

int subgroupBroadcast_1d79c7() {
  int arg_0 = 1;
  int res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_1d79c7();
}

DXC validation failure:
hlsl.hlsl:9:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = subgroupBroadcast_1d79c7();
  ^

