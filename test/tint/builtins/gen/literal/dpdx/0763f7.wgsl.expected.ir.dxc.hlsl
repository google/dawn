SKIP: FAILED

float3 dpdx_0763f7() {
  float3 res = ddx((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdx_0763f7();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdx_0763f7();
  ^

