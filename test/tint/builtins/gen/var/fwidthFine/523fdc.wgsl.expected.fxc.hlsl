float3 tint_fwidth_fine(float3 v) {
  return (abs(ddx_fine(v)) + abs(ddy_fine(v)));
}

RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthFine_523fdc() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = tint_fwidth_fine(arg_0);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  fwidthFine_523fdc();
  return;
}
