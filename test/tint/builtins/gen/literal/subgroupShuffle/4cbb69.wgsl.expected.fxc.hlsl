SKIP: INVALID

//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupShuffle_4cbb69() {
  uint3 res = WaveReadLaneAt((1u).xxx, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupShuffle_4cbb69());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupShuffle_4cbb69() {
  uint3 res = WaveReadLaneAt((1u).xxx, int(1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupShuffle_4cbb69());
}

