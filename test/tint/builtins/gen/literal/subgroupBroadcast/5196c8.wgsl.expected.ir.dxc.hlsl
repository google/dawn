SKIP: FAILED

float2 subgroupBroadcast_5196c8() {
  float2 res = WaveReadLaneAt((1.0f).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce = subgroupBroadcast_5196c8();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = subgroupBroadcast_5196c8();
  ^

