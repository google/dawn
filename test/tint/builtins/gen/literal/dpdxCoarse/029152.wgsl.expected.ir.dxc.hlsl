SKIP: FAILED

float dpdxCoarse_029152() {
  float res = ddx_coarse(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxCoarse_029152();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxCoarse_029152();
  ^

