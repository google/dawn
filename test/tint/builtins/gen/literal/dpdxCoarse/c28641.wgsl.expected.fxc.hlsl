float4 dpdxCoarse_c28641() {
  float4 res = ddx_coarse((1.0f).xxxx);
  return res;
}

RWByteAddressBuffer prevent_dce : register(u0);

void fragment_main() {
  prevent_dce.Store4(0u, asuint(dpdxCoarse_c28641()));
  return;
}
