
RWByteAddressBuffer prevent_dce : register(u0);
float3 fwidthFine_523fdc() {
  float3 v = ddx_fine((1.0f).xxx);
  float3 v_1 = ddy_fine((1.0f).xxx);
  float3 v_2 = abs(v);
  float3 res = (v_2 + abs(v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(fwidthFine_523fdc()));
}

