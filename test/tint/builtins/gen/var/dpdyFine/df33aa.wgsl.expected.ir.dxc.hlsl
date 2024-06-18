SKIP: FAILED

float2 dpdyFine_df33aa() {
  float2 arg_0 = (1.0f).xx;
  float2 res = ddy_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyFine_df33aa();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdyFine_df33aa();
  ^

