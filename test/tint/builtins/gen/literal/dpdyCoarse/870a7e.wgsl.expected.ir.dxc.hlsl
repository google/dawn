SKIP: FAILED

float dpdyCoarse_870a7e() {
  float res = ddy_coarse(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdyCoarse_870a7e();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdyCoarse_870a7e();
  ^

