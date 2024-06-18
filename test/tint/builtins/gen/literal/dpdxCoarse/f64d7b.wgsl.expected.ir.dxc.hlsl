SKIP: FAILED

float3 dpdxCoarse_f64d7b() {
  float3 res = ddx_coarse((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce = dpdxCoarse_f64d7b();
}

DXC validation failure:
hlsl.hlsl:7:3: error: use of undeclared identifier 'prevent_dce'
  prevent_dce = dpdxCoarse_f64d7b();
  ^

