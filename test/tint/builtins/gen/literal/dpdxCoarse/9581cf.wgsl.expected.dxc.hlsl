RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdxCoarse_9581cf() {
  float2 res = ddx_coarse((1.0f).xx);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  dpdxCoarse_9581cf();
  return;
}
