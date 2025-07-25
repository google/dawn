//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 quadBroadcast_0cc513() {
  float3 res = QuadReadLaneAt((1.0f).xxx, 1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_0cc513()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float3 quadBroadcast_0cc513() {
  float3 res = QuadReadLaneAt((1.0f).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadBroadcast_0cc513()));
}

