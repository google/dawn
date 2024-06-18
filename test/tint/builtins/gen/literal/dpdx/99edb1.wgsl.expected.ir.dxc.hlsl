SKIP: FAILED

float2 dpdx_99edb1() {
  float2 res = ddx((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_99edb1();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdx_99edb1();
  ^

