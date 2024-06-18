SKIP: FAILED

int3 subgroupBroadcast_e275c8() {
  int3 arg_0 = (1).xxx;
  int3 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_e275c8();
}

DXC validation failure:
hlsl.hlsl:9:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = subgroupBroadcast_e275c8();
  ^

