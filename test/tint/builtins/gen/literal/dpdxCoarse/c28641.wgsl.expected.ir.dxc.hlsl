SKIP: FAILED

float4 dpdxCoarse_c28641() {
  float4 res = ddx_coarse((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxCoarse_c28641();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxCoarse_c28641();
  ^

