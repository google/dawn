
RWByteAddressBuffer prevent_dce : register(u0);
float2 fwidthFine_ff6aa0() {
  float2 arg_0 = (1.0f).xx;
  float2 v = arg_0;
  float2 v_1 = ddx_fine(v);
  float2 v_2 = ddy_fine(v);
  float2 v_3 = abs(v_1);
  float2 res = (v_3 + abs(v_2));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(fwidthFine_ff6aa0()));
}

