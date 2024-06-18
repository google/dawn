SKIP: FAILED

float4 dpdxCoarse_c28641() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddx_coarse(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxCoarse_c28641();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxCoarse_c28641();
  ^

