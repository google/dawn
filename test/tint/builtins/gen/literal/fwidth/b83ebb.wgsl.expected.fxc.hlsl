RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidth_b83ebb() {
  float2 res = fwidth((1.0f).xx);
  prevent_dce.Store2(0u, asuint(res));
}

void fragment_main() {
  fwidth_b83ebb();
  return;
}
