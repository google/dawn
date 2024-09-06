
RWByteAddressBuffer prevent_dce : register(u0);
float2 fwidthFine_ff6aa0() {
  float2 v = ddx_fine((1.0f).xx);
  float2 v_1 = ddy_fine((1.0f).xx);
  float2 v_2 = abs(v);
  float2 res = (v_2 + abs(v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(fwidthFine_ff6aa0()));
}

