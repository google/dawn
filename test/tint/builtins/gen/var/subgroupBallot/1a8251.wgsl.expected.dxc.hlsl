//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBallot_1a8251() {
  bool arg_0 = true;
  uint4 res = WaveActiveBallot(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupBallot_1a8251());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupBallot_1a8251() {
  bool arg_0 = true;
  uint4 res = WaveActiveBallot(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupBallot_1a8251());
}

