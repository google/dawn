SKIP: FAILED

float4 dpdxFine_8c5069() {
  float4 arg_0 = (1.0f).xxxx;
  float4 res = ddx_fine(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_8c5069();
}

DXC validation failure:
hlsl.hlsl:8:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxFine_8c5069();
  ^

