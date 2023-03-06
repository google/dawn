RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthFine_523fdc() {
  float3 res = fwidth((1.0f).xxx);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  fwidthFine_523fdc();
  return;
}
