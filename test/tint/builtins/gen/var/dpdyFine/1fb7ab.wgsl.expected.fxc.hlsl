RWByteAddressBuffer prevent_dce : register(u0, space2);

void dpdyFine_1fb7ab() {
  float3 arg_0 = (1.0f).xxx;
  float3 res = ddy_fine(arg_0);
  prevent_dce.Store3(0u, asuint(res));
}

void fragment_main() {
  dpdyFine_1fb7ab();
  return;
}
