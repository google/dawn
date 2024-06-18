SKIP: FAILED

float3 dpdxFine_f92fb6() {
  float3 res = ddx_fine((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxFine_f92fb6();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxFine_f92fb6();
  ^

