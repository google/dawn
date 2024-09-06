
RWByteAddressBuffer prevent_dce : register(u0);
float4 fwidthFine_68f4ef() {
  float4 v = ddx_fine((1.0f).xxxx);
  float4 v_1 = ddy_fine((1.0f).xxxx);
  float4 v_2 = abs(v);
  float4 res = (v_2 + abs(v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(fwidthFine_68f4ef()));
}

