//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupShuffle_d4a772() {
  int res = WaveReadLaneAt(int(1), 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_d4a772()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int subgroupShuffle_d4a772() {
  int res = WaveReadLaneAt(int(1), 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffle_d4a772()));
}

