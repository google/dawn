SKIP: FAILED

uint2 subgroupBroadcast_4a4334() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_4a4334();
}

DXC validation failure:
hlsl.hlsl:9:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = subgroupBroadcast_4a4334();
  ^

