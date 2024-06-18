SKIP: FAILED

float4 dpdyCoarse_445d24() {
  float4 res = ddy_coarse((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyCoarse_445d24();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdyCoarse_445d24();
  ^

