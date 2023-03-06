RWByteAddressBuffer prevent_dce : register(u0, space2);

void fwidth_d2ab9a() {
  float4 res = fwidth((1.0f).xxxx);
  prevent_dce.Store4(0u, asuint(res));
}

void fragment_main() {
  fwidth_d2ab9a();
  return;
}
