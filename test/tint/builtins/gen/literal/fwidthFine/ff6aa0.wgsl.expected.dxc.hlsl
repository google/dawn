RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidthFine_ff6aa0() {
  float2 res = fwidth((1.0f).xx);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  fwidthFine_ff6aa0();
  return;
}
